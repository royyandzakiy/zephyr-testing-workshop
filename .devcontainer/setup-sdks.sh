#!/usr/bin/env bash
# postStartCommand: ensure the Zephyr version this project wants is in the shared
# volume, then register it.
#
# The SDK and Zephyr tree are not in the image -- they live in zephyr-sdks-cache,
# shared by name with every project that copies this .devcontainer/. So the first
# start on a machine takes ~10 min; every start after that, in any project, takes
# seconds and no network.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=versions.env
source "$HERE/versions.env"

STORE=/workdir/zephyr-sdks
ZEPHYR_WS="$STORE/$ZEPHYR_VERSION"
SDK_DIR="$STORE/toolchains/zephyr-sdk-$ZSDK_VERSION"

# The versions live here and in devcontainer.json (which the extension host reads
# and which cannot see an env file). Two places is fine; two places that disagree
# would provision one version and build against another.
desync=0
if [ -n "${ZEPHYR_BASE:-}" ] && [ "$ZEPHYR_BASE" != "$ZEPHYR_WS/zephyr" ]; then
    echo "ERROR: ZEPHYR_BASE does not match versions.env" >&2
    echo "  devcontainer.json says: $ZEPHYR_BASE" >&2
    echo "  versions.env implies:   $ZEPHYR_WS/zephyr" >&2
    desync=1
fi
if [ -n "${ZEPHYR_SDK_INSTALL_DIR:-}" ] && [ "$ZEPHYR_SDK_INSTALL_DIR" != "$SDK_DIR" ]; then
    echo "ERROR: ZEPHYR_SDK_INSTALL_DIR does not match versions.env" >&2
    echo "  devcontainer.json says: $ZEPHYR_SDK_INSTALL_DIR" >&2
    echo "  versions.env implies:   $SDK_DIR" >&2
    desync=1
fi
if [ "$desync" -ne 0 ]; then
    echo >&2
    echo "Fix .devcontainer/devcontainer.json (and macos/devcontainer.json) to match" >&2
    echo ".devcontainer/versions.env, then rebuild the container." >&2
    exit 1
fi

mkdir -p "$STORE"

# Two containers starting at once must not both populate the store -- concurrent
# west updates into one directory produce a corrupt workspace.
exec 9>"$STORE/.lock"
if ! flock -w 1800 9; then
    echo "ERROR: timed out waiting for another container to finish populating $STORE" >&2
    exit 1
fi

# --- Populate on first use ---------------------------------------------------
need_zephyr=0
need_sdk=0
[ -f "$ZEPHYR_WS/.complete" ] || need_zephyr=1
[ -f "$SDK_DIR/.complete" ] || need_sdk=1

if [ "$need_zephyr" -eq 1 ] || [ "$need_sdk" -eq 1 ]; then
    cat <<BANNER

  ============================================================================
   FIRST RUN ON THIS MACHINE

   Downloading Zephyr $ZEPHYR_VERSION and Zephyr SDK $ZSDK_VERSION into the
   shared volume. Takes roughly 10 minutes and needs a network connection.

   Once per machine, not once per project -- every later project using these
   versions starts in seconds.
  ============================================================================

BANNER
    ZEPHYR_STORE_LOCKED=1 FETCH_SKIP_REGISTER=1 \
        bash "$HERE/fetch-zephyr.sh" \
            "$([ "$need_zephyr" -eq 1 ] && echo "$ZEPHYR_VERSION" || echo "")" \
            "$([ "$need_sdk" -eq 1 ] && echo "$ZSDK_VERSION" || echo "")"
fi

# --- Verify ------------------------------------------------------------------
echo "=== Verifying Zephyr environment ==="

fail=0
if [ -f "$ZEPHYR_WS/zephyr/VERSION" ]; then
    echo "  Zephyr:      $ZEPHYR_WS/zephyr"
else
    echo "  ERROR: no Zephyr checkout at $ZEPHYR_WS/zephyr" >&2
    fail=1
fi

# hal/espressif is fetched late in west update, so its absence hints at a
# truncated one.
if [ ! -d "$ZEPHYR_WS/modules/hal/espressif" ]; then
    echo "  WARNING: $ZEPHYR_WS/modules/hal/espressif missing -- west update may be incomplete" >&2
fi

if [ -d "$SDK_DIR" ]; then
    echo "  Zephyr SDK:  $SDK_DIR"
    for t in $ZSDK_TOOLCHAINS; do
        if [ -d "$SDK_DIR/$t" ]; then
            echo "               $t"
        else
            echo "  ERROR: toolchain $t not installed in $SDK_DIR" >&2
            fail=1
        fi
    done
else
    echo "  ERROR: no Zephyr SDK at $SDK_DIR" >&2
    fail=1
fi

if [ "$fail" -ne 0 ]; then
    cat >&2 <<'HINT'

The shared volume is incomplete. Delete the broken piece and reopen the
container -- the .complete sentinels mean only that part is refetched:

    rm -rf /workdir/zephyr-sdks/<version>            # or .../toolchains/zephyr-sdk-<ver>

Or start over entirely (full ~10 min download):

    docker volume rm zephyr-sdks-cache

HINT
    exit 1
fi

# --- Re-register on every start ----------------------------------------------
# ~/.cmake is in the container's throwaway layer even though the SDK persists, so
# without this the nRF Connect SDK picker is empty after a rebuild.
echo "=== Setting up Zephyr SDK host tools ==="
(cd "$SDK_DIR" && ./setup.sh -h -c > /dev/null)

bash "$HERE/register-sdks.sh"
