#!/usr/bin/env bash
# Build the base -> ci -> devel image chain.
#
#     bash .devcontainer/build.sh [tag] [platform]
#
# devcontainer.json inlines the same three commands in initializeCommand (it
# cannot call this file -- see the comment there), so keep the two in step.
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
