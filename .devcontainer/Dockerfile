FROM zephyrprojectrtos/zephyr-build:v0.29.3

USER root

ARG DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# ----------- SETTING UP TOOLS

# 1. Base dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl wget unzip jq file \
    clang-format clangd gcc-multilib \
    libffi8 libyaml-0-2 \
    && rm -rf /var/lib/apt/lists/*

# 2. nrfutil & plugins
RUN curl -fSL "https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil" -o /usr/local/bin/nrfutil \
    && chmod +x /usr/local/bin/nrfutil \
    && nrfutil install toolchain-manager \
    && nrfutil install device

# 3. SEGGER J-Link runtime
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

# 4. Fail the build now, not in CI, if the wrong arch slipped in
RUN JLINK_LIB="$(ldconfig -p | awk '/libjlinkarm\.so\.7/ {print $NF; exit}')" && \
    echo "Resolved libjlinkarm: ${JLINK_LIB}" && \
    file -L "${JLINK_LIB}" | grep -q 'ELF 64-bit' && \
    nrfutil device --version

# ----------- SETTING UP GITHUB ACTIONS RUNNER
WORKDIR /actions-runner

RUN curl -o actions-runner-linux-x64-2.336.0.tar.gz -L https://github.com/actions/runner/releases/download/v2.336.0/actions-runner-linux-x64-2.336.0.tar.gz \
    && echo "04cf0be1aff4c3ec3554466c39124ca250e3effd8873bb7e8d68535aa9505d5d  actions-runner-linux-x64-2.336.0.tar.gz" | shasum -a 256 -c \
    && tar xzf ./actions-runner-linux-x64-2.336.0.tar.gz \
    && rm ./actions-runner-linux-x64-2.336.0.tar.gz

# ----------- SETTING UP USE/RESET DEFAULT ZEPHYR ENVIRONMENT VIA .BASHRC

# Example Usage:
#   use-vanilla v4.2.2 # Switches to Vanilla Zephyr v4.2.2
#   use-ncs v3.3.0     # Switches PATH and env vars to nRF Connect SDK v3.3.0
#   reset-ncs          # Restores clean baseline environment

# Append SDK environment helpers to bashrc
RUN cat << 'EOF' >> /root/.bashrc

if [ -z "$_ORIG_PATH" ]; then
    export _ORIG_PATH="$PATH"
    export _ORIG_LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
fi

use-ncs() {
    # default values
    local target_ver="${1:-v3.3.0}"
    local script_path
    
    # 1. Clear out any prior Vanilla or NCS environment overrides
    reset-ncs > /dev/null

    script_path=$(find /workspaces -name "ncs.py" 2>/dev/null | head -n 1)

    if [ -n "$script_path" ] && [ -f "$script_path" ]; then
        eval "$(/usr/bin/python3 "$script_path" "$target_ver")"
    else
        echo "Error: Could not locate ncs.py script." >&2
        return 1
    fi
}

use-vanilla() {
    # default values
    local target_ver="${1:-v4.2.2}"
    local sdk_ver="${2:-0.17.0}"
    
    # 1. Clear out prior environment overrides
    reset-ncs > /dev/null
    
    # 2. Export paths mirroring NCS layout
    export ZEPHYR_BASE="/workdir/zephyr-sdks/${target_ver}/zephyr"
    export ZEPHYR_SDK_INSTALL_DIR="/workdir/zephyr-sdks/toolchains/zephyr-sdk-${sdk_ver}"
    
    if [ -f "$ZEPHYR_BASE/zephyr-env.sh" ]; then
        source "$ZEPHYR_BASE/zephyr-env.sh"
        source "$ZEPHYR_SDK_INSTALL_DIR/setup.sh" -h -c -t x86_64-zephyr-elf -t arm-zephyr-eabi
        echo "Switched to Vanilla Zephyr ${target_ver} (SDK: zephyr-sdk-${sdk_ver})"
    else
        echo "Error: Vanilla Zephyr not found at $ZEPHYR_BASE" >&2
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

# Set default environment on interactive load, change if preferred
use-vanilla v4.2.2 0.17.0
EOF
