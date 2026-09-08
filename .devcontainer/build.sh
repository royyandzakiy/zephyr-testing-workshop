#!/usr/bin/env bash
# Build the three-stage image chain, the way zephyrproject-rtos/docker-image does:
#
#     Dockerfile.base   -> zephyr-workshop-base    build tools, no Zephyr SDK
#     Dockerfile.ci     -> zephyr-workshop-ci      + flashing tools, Actions runner
#     Dockerfile.devel  -> zephyr-workshop-devel   + editor tooling, shell helpers
#
# devcontainer.json runs this from initializeCommand, because a devcontainer can
# only build one Dockerfile itself. Every stage is layer-cached, so this is a
# no-op once built.
#
#     bash .devcontainer/build.sh [tag] [platform]
#
# Run it by hand to rebuild after editing a Dockerfile.
set -euo pipefail

cd "$(dirname "$0")"

TAG="${1:-local}"
PLATFORM="${2:-}"
PLATFORM_ARG=()
[ -n "$PLATFORM" ] && PLATFORM_ARG=(--platform "$PLATFORM")

build() {
    local file="$1" image="$2"
    shift 2
    echo "=== $file -> $image"
    docker build "${PLATFORM_ARG[@]}" -f "$file" -t "$image" "$@" .
}

build Dockerfile.base  "zephyr-workshop-base:${TAG}"
build Dockerfile.ci    "zephyr-workshop-ci:${TAG}"    --build-arg "BASE_IMAGE=zephyr-workshop-base:${TAG}"
build Dockerfile.devel "zephyr-workshop-devel:${TAG}" --build-arg "BASE_IMAGE=zephyr-workshop-ci:${TAG}"

echo "=== ready: zephyr-workshop-devel:${TAG}"
