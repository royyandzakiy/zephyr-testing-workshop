#!/usr/bin/env bash
# Install a vanilla Zephyr version and/or a Zephyr SDK into the shared volume.
# setup-sdks.sh calls this on first container start; run it by hand to add more:
#
#     bash .devcontainer/fetch-zephyr.sh v4.3.0 0.17.4   # then: use-vanilla v4.3.0 0.17.4
#
# An omitted argument falls back to versions.env; an empty one ("") skips that half.
#
# .complete sentinels are written only after a step fully succeeds -- a guard on
# directory existence would report a half-dead `west update` as ready forever.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=versions.env
source "$HERE/versions.env"

ZEPHYR_VER="${1-$ZEPHYR_VERSION}"
SDK_VER="${2-$ZSDK_VERSION}"

if [ -z "$ZEPHYR_VER" ] && [ -z "$SDK_VER" ]; then
    echo "usage: $(basename "$0") <zephyr-version> [sdk-version]" >&2
    exit 2
fi

STORE=/workdir/zephyr-sdks
mkdir -p "$STORE"

# setup-sdks.sh already holds this lock when it calls us; taking it again from a
# child process would deadlock.
if [ -z "${ZEPHYR_STORE_LOCKED:-}" ]; then
    exec 9>"$STORE/.lock"
    flock -w 1800 9 || { echo "ERROR: timed out waiting for the $STORE lock" >&2; exit 1; }
fi

# --- Zephyr SDK --------------------------------------------------------------
# `setup.sh -t` is incremental, so re-running against an installed SDK just adds
# any newly listed toolchain.
if [ -n "$SDK_VER" ]; then
    SDK_DIR="$STORE/toolchains/zephyr-sdk-$SDK_VER"
    targs=()
    for t in $ZSDK_TOOLCHAINS; do targs+=(-t "$t"); done

    if [ -f "$SDK_DIR/.complete" ]; then
        echo "=== Zephyr SDK $SDK_VER present -- ensuring toolchains are installed ==="
        (cd "$SDK_DIR" && ./setup.sh -h -c "${targs[@]}")
    else
        echo "=== Installing Zephyr SDK $SDK_VER into $STORE/toolchains ==="
        rm -rf "$SDK_DIR"
        mkdir -p "$STORE/toolchains"
        wget -qO- "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/zephyr-sdk-${SDK_VER}_linux-x86_64_minimal.tar.xz" \
            | tar -xJ -C "$STORE/toolchains"
        (cd "$SDK_DIR" && ./setup.sh -h -c "${targs[@]}")
        touch "$SDK_DIR/.complete"
    fi
fi

# --- Vanilla Zephyr west workspace -------------------------------------------
if [ -n "$ZEPHYR_VER" ]; then
    WS="$STORE/$ZEPHYR_VER"
    if [ -f "$WS/.complete" ]; then
        echo "=== Vanilla Zephyr $ZEPHYR_VER already installed ==="
    else
        echo "=== Fetching Vanilla Zephyr $ZEPHYR_VER into $WS ==="
        rm -rf "$WS"
        mkdir -p "$WS"
        git clone --depth 1 --branch "$ZEPHYR_VER" \
            https://github.com/zephyrproject-rtos/zephyr.git "$WS/zephyr"
        cd "$WS"
        west init -l "$WS/zephyr"
        west update --narrow -o=--depth=1
        west zephyr-export
        for m in ${ZEPHYR_BLOBS:-}; do
            echo "=== Fetching binary blobs for $m ==="
            west blobs fetch "$m"
        done
        touch "$WS/.complete"
    fi
fi

if [ -z "${FETCH_SKIP_REGISTER:-}" ]; then
    bash "$HERE/register-sdks.sh"
    echo
    echo "Done. Switch to it with:"
    echo "    use-vanilla ${ZEPHYR_VER:-<version>} ${SDK_VER:-$ZSDK_VERSION}"
fi
