# Zephyr Testing Workshop - development container (~3.5 GB).
#
# Ground-up from Ubuntu instead of zephyrprojectrtos/zephyr-build (~32 GB, mostly
# toolchains and emulators this repo never uses).
#
# The Zephyr SDK and the Zephyr tree are NOT in this image. They live in the
# shared volume at /workdir/zephyr-sdks and are installed on first container
# start by setup-sdks.sh, so every project that copies this .devcontainer/ reuses
# one download per machine.
#
# versions.env is the only file that differs between projects. Keep this
# Dockerfile byte-identical everywhere so the layer cache keeps hitting.
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

# --- 1. OS packages ----------------------------------------------------------
# gcc-multilib/g++-multilib are load-bearing: native_sim is a 32-bit host binary
# and these pull in /lib32/libasan.so.* and /lib32/libubsan.so.*, which
# sanitizers_native-sim_ci.yml asserts are present.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-multilib \
    g++-multilib \
    cmake \
    ninja-build \
    make \
    gperf \
    ccache \
    device-tree-compiler \
    gdb \
    gcovr \
    lcov \
    clang-format \
    clangd \
    libsdl2-dev \
    python3 \
    python3-dev \
    python3-venv \
    python3-pip \
    python3-setuptools \
    python-is-python3 \
    dfu-util \
    picocom \
    usbutils \
    libusb-1.0-0 \
    libffi8 \
    libyaml-0-2 \
    ca-certificates \
    curl \
    wget \
    git \
    gnupg \
    openssh-client \
    file \
    jq \
    unzip \
    xz-utils \
    rsync \
    socat \
    srecord \
    sudo \
    locales \
    && rm -rf /var/lib/apt/lists/*

# --- 2. Python virtualenv ----------------------------------------------------
# Ubuntu 24.04 marks the system interpreter externally-managed (PEP 668), so a
# venv is required, not stylistic.
ENV VIRTUAL_ENV=/opt/python-venv
ENV PATH=${VIRTUAL_ENV}/bin:${PATH}

RUN python3 -m venv ${VIRTUAL_ENV} \
    && pip install --no-cache-dir --upgrade pip setuptools wheel \
    && pip install --no-cache-dir west

# --- 3. Second SDK store (empty) ---------------------------------------------
# use-vanilla searches /workdir first, then here. Left empty so a future
# prebuilt/GHCR image can bake a default pair in without touching the helpers.
RUN mkdir -p /opt/zephyr-sdks

# --- 4. Zephyr Python requirements -------------------------------------------
# One venv for all Zephyr versions. These pins barely move between minor
# releases; bump ZEPHYR_REQ_REF if a newer Zephyr needs something. Independent of
# versions.env. requirements-doc is excluded (sphinx, several hundred MB).
ARG ZEPHYR_REQ_REF=v4.2.2
RUN ZREQ="https://raw.githubusercontent.com/zephyrproject-rtos/zephyr/${ZEPHYR_REQ_REF}/scripts" \
    && pip install --no-cache-dir \
      -r "${ZREQ}/requirements-base.txt" \
      -r "${ZREQ}/requirements-build-test.txt" \
      -r "${ZREQ}/requirements-run-test.txt" \
      -r "${ZREQ}/requirements-extras.txt" \
      'esptool>=5.0.2'

# --- 5. nrfutil --------------------------------------------------------------
RUN curl -fSL "https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil" -o /usr/local/bin/nrfutil \
    && chmod +x /usr/local/bin/nrfutil \
    && nrfutil install toolchain-manager \
    && nrfutil install device

# --- 6. SEGGER J-Link runtime ------------------------------------------------
ARG NORDIC_COMMAND_LINE_TOOLS_VERSION="10-24-0/nrf-command-line-tools-10.24.0"
RUN NCLT_BASE=https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x && \
    NCLT_TMP=$(mktemp -d) && cd "${NCLT_TMP}" && \
    wget -qO - "${NCLT_BASE}/${NORDIC_COMMAND_LINE_TOOLS_VERSION}_linux-amd64.tar.gz" | tar --no-same-owner -xz && \
    mkdir -p /opt/SEGGER && \
    tar xzf JLink_Linux_*x86_64.tgz -C /opt/SEGGER && \
    mv /opt/SEGGER/JLink_Linux* /opt/SEGGER/JLink && \
    rm -rf /opt/SEGGER/JLink/x86 /opt/SEGGER/JLink/libjlinkarm_x86.so* && \
    cd /opt/SEGGER/JLink && \
    JLINK_SO="$(ls libjlinkarm.so.[0-9]*.[0-9]*.[0-9]* | head -n1)" && \
    echo "Linking against ${JLINK_SO}" && \
    ln -sf "${JLINK_SO}" libjlinkarm.so.7 && \
    ln -sf libjlinkarm.so.7 libjlinkarm.so && \
    ln -s /opt/SEGGER/JLink/JLinkExe /usr/local/bin/JLinkExe && \
    echo "/opt/SEGGER/JLink" > /etc/ld.so.conf.d/SEGGER_JLink.conf && ldconfig && \
    cd / && rm -rf "${NCLT_TMP}"

# --- 7. Build-time asserts ---------------------------------------------------
# Fail here rather than on someone's first flash or first sanitizer run.
RUN JLINK_LIB="$(ldconfig -p | awk '/libjlinkarm\.so\.7/ {print $NF; exit}')" && \
    file -L "${JLINK_LIB}" | grep -q 'ELF 64-bit' && \
    nrfutil device --version

RUN set -e; \
    ldconfig -p | grep -q "libasan.*lib32"  || { echo "MISSING 32-bit libasan"; exit 1; }; \
    ldconfig -p | grep -q "libubsan.*lib32" || { echo "MISSING 32-bit libubsan"; exit 1; }; \
    west --version && cmake --version | head -1 && \
    python3 -c "import pytest; print('pytest', pytest.__version__)"

# --- 8. GitHub Actions runner ------------------------------------------------
WORKDIR /actions-runner

RUN curl -o actions-runner-linux-x64-2.336.0.tar.gz -L https://github.com/actions/runner/releases/download/v2.336.0/actions-runner-linux-x64-2.336.0.tar.gz \
    && echo "04cf0be1aff4c3ec3554466c39124ca250e3effd8873bb7e8d68535aa9505d5d  actions-runner-linux-x64-2.336.0.tar.gz" | sha256sum -c \
    && tar xzf ./actions-runner-linux-x64-2.336.0.tar.gz \
    && rm ./actions-runner-linux-x64-2.336.0.tar.gz

WORKDIR /workspaces

# --- 9. Shell helpers --------------------------------------------------------
#   use-vanilla [ver] [sdk]  switch this shell to a vanilla Zephyr + SDK
#   use-ncs [ver]            switch to an nRF Connect SDK install
#   reset-ncs                back to the container baseline
#   zephyr-stores            list what is installed, and where
RUN cat << 'EOF' >> /root/.bashrc

if [ -z "$_ORIG_PATH" ]; then
    export _ORIG_PATH="$PATH"
    export _ORIG_LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
    # reset-ncs unsets the live vars; these keep the project default recoverable.
    export _ORIG_ZEPHYR_BASE="${ZEPHYR_BASE:-}"
    export _ORIG_ZEPHYR_SDK_INSTALL_DIR="${ZEPHYR_SDK_INSTALL_DIR:-}"
fi

# Searched in order: shared volume first, then the image's (empty) store.
ZEPHYR_STORES=(/workdir/zephyr-sdks /opt/zephyr-sdks)

# Version numbers are derived from the paths devcontainer.json exported, never
# hardcoded -- a literal here would override versions.env in every project that
# copies this file.
#   ZEPHYR_BASE=<store>/v4.2.2/zephyr                    -> v4.2.2
#   ZEPHYR_SDK_INSTALL_DIR=<store>/.../zephyr-sdk-0.17.0 -> 0.17.0
_zephyr_env_version() {
    local b="${ZEPHYR_BASE:-${_ORIG_ZEPHYR_BASE:-}}"
    [ -n "$b" ] || return 0
    basename "$(dirname "$b")"
}

_zephyr_env_sdk_version() {
    local s="${ZEPHYR_SDK_INSTALL_DIR:-${_ORIG_ZEPHYR_SDK_INSTALL_DIR:-}}"
    [ -n "$s" ] || return 0
    basename "$s" | sed 's/^zephyr-sdk-//'
}

zephyr-stores() {
    local root p
    echo "Zephyr workspaces:"
    for root in "${ZEPHYR_STORES[@]}"; do
        for p in "$root"/*/zephyr/VERSION; do
            [ -f "$p" ] && echo "  $(basename "$(dirname "$(dirname "$p")")")  ->  $(dirname "$p")"
        done
    done
    echo "Zephyr SDKs:"
    for root in "${ZEPHYR_STORES[@]}"; do
        for p in "$root"/toolchains/zephyr-sdk-*; do
            [ -d "$p" ] && echo "  $(basename "$p")  ->  $p"
        done
    done
}

use-vanilla() {
    local target_ver="${1:-$(_zephyr_env_version)}"
    local sdk_ver="${2:-$(_zephyr_env_sdk_version)}"
    local root base="" sdk=""

    if [ -z "$target_ver" ] || [ -z "$sdk_ver" ]; then
        echo "Usage: use-vanilla <zephyr-version> <sdk-version>   e.g. use-vanilla v4.2.2 0.17.0" >&2
        echo "       (no default available -- ZEPHYR_BASE/ZEPHYR_SDK_INSTALL_DIR are unset)" >&2
        return 1
    fi

    # Resolve BEFORE reset-ncs. reset-ncs unsets ZEPHYR_BASE and
    # ZEPHYR_SDK_INSTALL_DIR, so bailing out after it would leave the shell with
    # no Zephyr environment at all -- and a missing ZEPHYR_SDK_INSTALL_DIR sends
    # FindZephyr-sdk.cmake down its registry-search path, which fails with an
    # unhelpful "Could not find a package configuration file".
    # Workspace and SDK resolve independently -- mixing stores is fine.
    for root in "${ZEPHYR_STORES[@]}"; do
        [ -z "$base" ] && [ -d "$root/$target_ver/zephyr" ] && base="$root/$target_ver/zephyr"
        [ -z "$sdk" ] && [ -d "$root/toolchains/zephyr-sdk-$sdk_ver" ] && sdk="$root/toolchains/zephyr-sdk-$sdk_ver"
    done

    if [ -z "$base" ]; then
        echo "Error: Zephyr $target_ver not found in ${ZEPHYR_STORES[*]}" >&2
        echo "       Install it with: bash .devcontainer/fetch-zephyr.sh $target_ver" >&2
        echo "       Environment left unchanged." >&2
        return 1
    fi
    if [ -z "$sdk" ]; then
        echo "Error: zephyr-sdk-$sdk_ver not found in ${ZEPHYR_STORES[*]}" >&2
        echo "       Install it with: bash .devcontainer/fetch-zephyr.sh $target_ver $sdk_ver" >&2
        echo "       Environment left unchanged." >&2
        return 1
    fi
    if [ ! -f "$sdk/cmake/Zephyr-sdkConfig.cmake" ]; then
        echo "Error: $sdk has no cmake/Zephyr-sdkConfig.cmake -- the SDK is incomplete." >&2
        echo "       Repair it with: rm -rf $sdk && bash .devcontainer/setup-sdks.sh" >&2
        echo "       Environment left unchanged." >&2
        return 1
    fi

    reset-ncs > /dev/null

    export ZEPHYR_BASE="$base"
    export ZEPHYR_SDK_INSTALL_DIR="$sdk"
    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

    source "$ZEPHYR_BASE/zephyr-env.sh"
    source "$ZEPHYR_SDK_INSTALL_DIR/setup.sh" -h -c > /dev/null
    echo "Switched to Vanilla Zephyr ${target_ver} (SDK: zephyr-sdk-${sdk_ver})"
    echo "  ZEPHYR_BASE=$ZEPHYR_BASE"
    echo "  ZEPHYR_SDK_INSTALL_DIR=$ZEPHYR_SDK_INSTALL_DIR"
}

use-ncs() {
    local target_ver="${1:-v3.3.0}"
    local script_path

    reset-ncs > /dev/null

    script_path=$(find /workspaces -name "ncs.py" 2>/dev/null | head -n 1)

    if [ -n "$script_path" ] && [ -f "$script_path" ]; then
        eval "$(/usr/bin/python3 "$script_path" "$target_ver")"
    else
        echo "Error: Could not locate ncs.py script." >&2
        return 1
    fi
}

reset-ncs() {
    export PATH="$_ORIG_PATH"
    if [ -n "$_ORIG_LD_LIBRARY_PATH" ]; then
        export LD_LIBRARY_PATH="$_ORIG_LD_LIBRARY_PATH"
    else
        unset LD_LIBRARY_PATH
    fi
    unset ZEPHYR_BASE ZEPHYR_SDK_INSTALL_DIR NCS_TOOLCHAIN_PATH ZEPHYR_TOOLCHAIN_VARIANT
    echo "Reset environment to container baseline."
}

# Project default. No version arguments on purpose -- see _zephyr_env_version.
use-vanilla
EOF
