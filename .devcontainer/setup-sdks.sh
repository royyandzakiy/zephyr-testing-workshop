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

# --- Adopt installs made before the sentinels existed -------------------------
# A volume provisioned by the old scripts has no .complete marker. Stamp one if
# the install genuinely looks finished, so upgrading does not trigger a pointless
# refetch. The checks are deliberately specific: a half-extracted SDK -- which the
# old `[ ! -d ... ]` guard would leave behind and then skip forever -- has the
# directory but neither setup.sh nor cmake/Zephyr-sdkConfig.cmake, so it is NOT
# adopted and gets refetched below.
if [ ! -f "$SDK_DIR/.complete" ] \
   && [ -f "$SDK_DIR/setup.sh" ] \
   && [ -f "$SDK_DIR/cmake/Zephyr-sdkConfig.cmake" ]; then
    echo "=== Adopting existing Zephyr SDK $ZSDK_VERSION ==="
    touch "$SDK_DIR/.complete"
fi
if [ ! -f "$ZEPHYR_WS/.complete" ] \
   && [ -f "$ZEPHYR_WS/zephyr/VERSION" ] \
   && [ -d "$ZEPHYR_WS/.west" ] \
   && [ -d "$ZEPHYR_WS/modules/hal/espressif" ]; then
    echo "=== Adopting existing Zephyr $ZEPHYR_VERSION workspace ==="
    touch "$ZEPHYR_WS/.complete"
fi

# --- Populate on first use ---------------------------------------------------
need_zephyr=0
need_sdk=0
fresh_install=0
[ -f "$ZEPHYR_WS/.complete" ] || { need_zephyr=1; fresh_install=1; }
[ -f "$SDK_DIR/.complete" ] || { need_sdk=1; fresh_install=1; }

# An installed SDK can still be missing a toolchain this project needs -- another
# project may have provisioned the shared store with a shorter ZSDK_TOOLCHAINS.
# fetch-zephyr.sh adds them incrementally, so ask for it rather than failing
# verification below.
if [ "$need_sdk" -eq 0 ]; then
    for t in $ZSDK_TOOLCHAINS; do
        if [ ! -d "$SDK_DIR/$t" ]; then
            echo "=== Toolchain $t missing from the shared SDK -- installing ==="
            need_sdk=1
        fi
    done
fi

if [ "$fresh_install" -eq 1 ]; then
    cat <<BANNER

  ============================================================================
   FIRST RUN ON THIS MACHINE

   Installing into the shared volume. Takes up to 10 minutes and needs a
   network connection. Once per machine, not once per project -- every later
   project using these versions starts in seconds.
  ============================================================================

BANNER
fi

if [ "$need_zephyr" -eq 1 ] || [ "$need_sdk" -eq 1 ]; then
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
    if [ ! -f "$SDK_DIR/cmake/Zephyr-sdkConfig.cmake" ]; then
        echo "  ERROR: $SDK_DIR/cmake/Zephyr-sdkConfig.cmake missing" >&2
        fail=1
    fi
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

# The registry is the only thing FindZephyr-sdk.cmake can fall back on when
# ZEPHYR_SDK_INSTALL_DIR is absent from a subprocess environment. If it is empty,
# a build fails with an opaque "Could not find a package configuration file
# provided by Zephyr-sdk" -- so say so here instead.
if ! ls "${HOME}/.cmake/packages/Zephyr-sdk/"* >/dev/null 2>&1; then
    echo "ERROR: no Zephyr-sdk entry in ${HOME}/.cmake/packages -- builds will fail" >&2
    echo "       to locate the SDK whenever ZEPHYR_SDK_INSTALL_DIR is not set." >&2
    exit 1
fi
