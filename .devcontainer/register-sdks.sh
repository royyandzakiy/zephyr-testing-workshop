#!/usr/bin/env bash
# Register every Zephyr/NCS workspace and Zephyr SDK under /workdir in the CMake
# *user package registry*, so the nRF Connect for VS Code extension lists them.
#
# WHY THIS EXISTS
#   The extension does not scan an install directory for SDKs. It reads
#   ~/.cmake/packages/<pkg>/* -- each file holds a path -- and derives:
#     Zephyr          -> <topdir>/zephyr/share/zephyr-package/cmake      (SDK picker)
#     Zephyr-sdk      -> <zephyr-sdk-x.y.z>/cmake                        (toolchain list)
#     ZephyrUnittest  -> <topdir>/zephyr/share/zephyrunittest-package/cmake
#   `west zephyr-export` and `zephyr-sdk-x.y.z/setup.sh -c` write those entries,
#   but neither runs on a warm container start, and ~/.cmake lives in the
#   container's throwaway layer (no volume) -- so the registration is lost on every
#   rebuild while the SDKs themselves survive in the /workdir volumes.
#
#   Run from setup-sdks.sh (postStartCommand) on every start. Safe to re-run, and
#   safe to run by hand if the extension's SDK picker ever comes up empty:
#       bash .devcontainer/register-sdks.sh
#   then: Command Palette -> "nRF Connect: Refresh SDKs" and "Refresh Toolchains".
#
# NOTE the /workdir/*/*/ glob covers BOTH stores (zephyr-sdks/v4.2.2 and
# ncs-sdks/v3.3.0) and skips nrfutil's toolchains/, downloads/ and tmp/ for free --
# those have no zephyr/share/... underneath them.
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
for p in /workdir/*/*/zephyr/share/zephyr-package/cmake; do
    register Zephyr "$p"
done

# unit_testing builds resolve find_package(ZephyrUnittest), not find_package(Zephyr).
for p in /workdir/*/*/zephyr/share/zephyrunittest-package/cmake; do
    register ZephyrUnittest "$p"
done

# Zephyr SDK toolchains (the NCS toolchains are found by nrfutil instead -- see
# nrf-connect.toolchainManager.installDirectory in devcontainer.json).
for p in /workdir/*/toolchains/zephyr-sdk-*/cmake; do
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
