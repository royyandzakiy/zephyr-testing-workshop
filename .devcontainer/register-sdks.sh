#!/usr/bin/env bash
# Register every Zephyr/NCS workspace and Zephyr SDK in the CMake *user package
# registry* (~/.cmake/packages), which is how the nRF Connect for VS Code
# extension discovers them -- it does not scan install directories.
#
# ~/.cmake is in the container's throwaway layer while the SDKs survive in the
# volumes, so this has to re-run on every start. Safe to run by hand if the SDK
# picker comes up empty, then Command Palette -> "nRF Connect: Refresh SDKs".
#
# /workdir/*/*/ covers both volume stores and skips nrfutil's toolchains/,
# downloads/ and tmp/ for free. /opt/zephyr-sdks is one level shallower, hence
# the second glob in each loop.
set -eu

REG_ROOT="${HOME}/.cmake/packages"

register() {
    local pkg="$1" path="$2" hash
    [ -d "$path" ] || return 0
    hash="$(printf '%s' "$path" | md5sum | cut -d' ' -f1)"
    mkdir -p "${REG_ROOT}/${pkg}"
    printf '%s' "$path" > "${REG_ROOT}/${pkg}/${hash}"
    echo "  ${pkg}: ${path}"
    count=$((count + 1))
}

echo "=== Registering SDKs for the nRF Connect extension ==="
count=0

# West workspaces -- vanilla Zephyr and NCS alike.
for p in /workdir/*/*/zephyr/share/zephyr-package/cmake \
         /opt/zephyr-sdks/*/zephyr/share/zephyr-package/cmake; do
    register Zephyr "$p"
done

# unit_testing builds resolve find_package(ZephyrUnittest), not find_package(Zephyr).
for p in /workdir/*/*/zephyr/share/zephyrunittest-package/cmake \
         /opt/zephyr-sdks/*/zephyr/share/zephyrunittest-package/cmake; do
    register ZephyrUnittest "$p"
done

# Zephyr SDK toolchains (the NCS toolchains are found by nrfutil instead -- see
# nrf-connect.toolchainManager.installDirectory in devcontainer.json).
for p in /workdir/*/toolchains/zephyr-sdk-*/cmake \
         /opt/zephyr-sdks/toolchains/zephyr-sdk-*/cmake; do
    register Zephyr-sdk "$p"
done

# Drop entries whose target no longer exists, so the picker does not offer SDKs
# that were deleted or lived in a previous image layer.
pruned=0
for f in "${REG_ROOT}"/*/*; do
    [ -f "$f" ] || continue
    if [ ! -d "$(cat "$f")" ]; then
        echo "  pruned stale: $(cat "$f")"
        rm -f "$f"
        pruned=$((pruned + 1))
    fi
done

echo "register-sdks: ${count} registration(s), ${pruned} stale entr(y/ies) pruned"
