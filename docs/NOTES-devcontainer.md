# Devcontainer Reference: Environment, Mounts, SDK Switching

**General Notes**:

- Four layers, each owning a different thing. Almost all "why isn't my change taking effect" confusion comes from editing the wrong layer.
- The Zephyr SDK is **not baked into the image**. It lives in a named Docker volume, provisioned on first start. This keeps the image small and lets it survive image rebuilds.
- The volume is **shared by name across every project** that copies this `.devcontainer/`. Zephyr and the SDK are downloaded once per *machine*, not once per project.
- The images are built ground-up from `ubuntu:24.04` as a three-stage chain, not derived from `zephyrprojectrtos/zephyr-build` (~32 GB, mostly toolchains and emulators this repo never uses).
- This repo is **not** a west manifest repo. There is no `west.yml`; `ZEPHYR_BASE` points outside the workspace into the SDK volume. That is why the env-var shell helpers exist at all. A `west.yml` here would mean west's T2 topology, which clones Zephyr *into each project's* workspace — exactly what the shared volume exists to avoid.

## The Four Layers

```
Dockerfile.base   → build tools, Python venv, Zephyr's Python requirements
  → Dockerfile.ci    → + nrfutil, J-Link, GitHub Actions runner (flashing / automation)
    → Dockerfile.devel  → + clangd, picocom, .bashrc helpers   ← the image you run
build.sh          → drives that chain (a devcontainer can only build one Dockerfile itself)
versions.env      → WHICH Zephyr + SDK + toolchains this project wants
devcontainer.json → how the image is launched (mounts, privileges, user) + editor config
setup-sdks.sh     → fills the volumes at runtime (with fetch-zephyr.sh)
```

Order of truth: the Dockerfiles define what exists → `devcontainer.json` decides how it is launched and what is mounted → `setup-sdks.sh` fills the volumes with whatever `versions.env` names. Missing tool, fix the right Dockerfile. Missing device or path, fix `devcontainer.json`. Wrong *version*, fix `versions.env` **and** the matching paths in `devcontainer.json` (which cannot read an env file — `setup-sdks.sh` fails loudly if the two disagree).

Rebuild cost follows the same order: Dockerfile change = `bash .devcontainer/build.sh` (slow, and a `Dockerfile.base` change rebuilds all three), `devcontainer.json` change = container recreate (fast), `setup-sdks.sh` or `versions.env` change = just restart (instant, volumes persist).

---

## `devcontainer.json`

```json
{
  "name": "Zephyr Development",
  "image": "zephyr-workshop-devel:local",
  "initializeCommand": ["bash", ".devcontainer/build.sh"],
  "containerEnv": {
    "RUNNER_ALLOW_RUNASROOT": "1",
    "ZEPHYR_BASE": "/workdir/zephyr-sdks/v4.2.2/zephyr",
    "ZEPHYR_SDK_INSTALL_DIR": "/workdir/zephyr-sdks/toolchains/zephyr-sdk-0.17.0",
    "ZEPHYR_TOOLCHAIN_VARIANT": "zephyr"
  },
  "containerUser": "root",
  "remoteUser": "root",
  "updateRemoteUserUID": false,
  "runArgs": ["--privileged"],
  "mounts": [ ... ],
  "workspaceFolder": "/workspaces/${localWorkspaceFolderBasename}",
  "postStartCommand": "/bin/bash .devcontainer/setup-sdks.sh"
}
```

### Why `image` + `initializeCommand`, not `build`

A devcontainer builds exactly **one** Dockerfile, and this repo has a three-stage chain. So `build.sh` builds it and `devcontainer.json` just names the finished image. `initializeCommand` runs on the **host** before the container exists, which is the only hook early enough. Every stage is layer-cached, so it is a no-op once built.

The cost: **the host needs `bash`**. Fine on macOS and Linux; on Windows this means Git Bash (bundled with Git for Windows). Without it the container will not start.

To rebuild by hand after editing a Dockerfile:

```bash
bash .devcontainer/build.sh                    # zephyr-workshop-{base,ci,devel}:local
bash .devcontainer/build.sh local-amd64 linux/amd64   # what macos/devcontainer.json uses
```

### Identity and privileges

- `containerUser`: user the container process runs as. Root here, because flashing needs raw USB access and the Actions runner writes to `/actions-runner`.
- `remoteUser`: user the editor server and its terminals run as. Usually matches `containerUser`.
- `updateRemoteUserUID`: normally the editor rewrites the container user's UID to match your host UID, so bind-mounted files are not root-owned. **Must be `false` when the user is already root** — otherwise the rewrite fails or mangles ownership.
- `containerEnv`: env vars set at container level, visible to every process including non-interactive shells. Unlike `.bashrc` exports, these survive into CI steps.
  - `RUNNER_ALLOW_RUNASROOT=1`: the GitHub Actions runner refuses to start as root without it. Only needed because of the self-hosted runner.
- `runArgs`: raw arguments passed straight to `docker run`.
  - `--privileged`: full device access. Required for USB probe passthrough (J-Link, ST-LINK, ESP32 native USB). Blunt instrument — `--device-cgroup-rule` is narrower but breaks on re-enumeration.

### Mounts

```json
"mounts": [
  "source=zephyr-sdks-cache,target=/workdir/zephyr-sdks,type=volume",
  "source=ncs-sdks-cache,target=/workdir/ncs-sdks,type=volume",
  "source=${localWorkspaceFolderBasename}-actions-runner,target=/actions-runner,type=volume",
  "source=/dev,target=/dev,type=bind,bind-propagation=rslave"
]
```

- `type=volume`: Docker-managed named volume. Survives container deletion and image rebuild. Use for anything expensive to re-download.
  - `zephyr-sdks-cache` — vanilla Zephyr source + Zephyr SDK toolchain
  - `ncs-sdks-cache` — NCS toolchains, managed by `nrfutil toolchain-manager`
  - `${localWorkspaceFolderBasename}-actions-runner` — runner registration and credentials, so you do not re-register on every rebuild
- **The naming asymmetry is deliberate.** The two SDK volumes have fixed names with no project prefix, so every project that copies this `.devcontainer/` mounts the *same* volumes and shares one download per machine. The runner volume is the opposite: it holds a single runner registration, so a shared name would make two projects fight over it.
- `type=bind`: maps a host path in. Changes are live in both directions.
- `bind-propagation=rslave`: **the important one**. When a board resets and USB re-enumerates, the device node number changes (`/dev/bus/usb/001/008` → `009`). Without `rslave` the container keeps the stale view and flashing fails with "cannot connect to J-Link" *after* the probe was already detected fine. `rslave` propagates host mount events inward.

### Workspace and lifecycle

- `workspaceFolder`: where the repo appears inside the container. `${localWorkspaceFolderBasename}` expands to the host folder name, giving `/workspaces/zephyr-testing-workshop`.
- Lifecycle hooks, in execution order:

| Hook | When | Use for |
|---|---|---|
| `initializeCommand` | on the **host**, before the container exists | host-side prep |
| `onCreateCommand` | once, at container creation | one-time per-container setup |
| `updateContentCommand` | at creation and on content updates | dependency refresh |
| `postCreateCommand` | once, after creation | `pip install -r requirements-dev.txt` |
| `postStartCommand` | **every start**, including restarts | `setup-sdks.sh` — must be idempotent |
| `postAttachCommand` | every time an editor attaches | editor-only fixups |

This repo uses **both**: `initializeCommand` to build the image chain on the host, and `postStartCommand` for `setup-sdks.sh`, which therefore has to be idempotent — it runs on every single start.

Idempotent here means `.complete` sentinel files, **not** `if [ ! -d ... ]` guards. A directory-existence check treats "the download started" as "the install finished": a `tar` that creates the directory and then dies leaves a hollow SDK that every later start reports as ready. That bug is exactly how this repo ended up with an SDK directory containing no SDK. Sentinels are written only after a step fully succeeds.

### Editor config

- `customizations.vscode.extensions`: auto-installed on attach. Purely editor-side, ignored by everything else.
  - `nordic-semiconductor.nrf-connect` — board and DTS tooling
- `customizations.vscode.settings`: `nrf-connect.toolchainManager.installDirectory` points the extension's bundled nrfutil at `/workdir/ncs-sdks`. See [`register-sdks.sh`](#register-sdkssh--why-the-nrf-connect-extension-sees-the-sdks).
  - `thecreativedodo.usbip-connect` — attaches WSL2 USB devices from inside the editor
  - `ms-vscode.cpptools` is installed, but `.clangd` is the config actually in use. Pick one — running IntelliSense and clangd together produces duplicate, contradictory diagnostics.

---

## Named Volumes

```bash
docker volume ls                                  # list
docker volume inspect zephyr-sdks-cache           # mountpoint + metadata
docker volume rm zephyr-sdks-cache                # force full re-provision on next start
docker volume prune                               # remove all unreferenced volumes
```

The container must be stopped before `rm` succeeds. After removing `zephyr-sdks-cache`, the next start re-downloads Zephyr and the SDK — roughly 10 minutes, so not something to do casually. Note this also affects **every other project** on the machine that shares the volume.

To see what is actually installed and which store it came from, run `zephyr-stores` inside the container.

Rebuild paths (Command Palette):

- **Dev Containers: Rebuild Container** — recreate container, keep image cache, keep volumes
- **Dev Containers: Rebuild Without Cache** — full Dockerfile re-run, keep volumes
- Volumes are only removed by `docker volume rm`, never by a rebuild.

---

## `setup-sdks.sh`

Runs on every container start. Reads `versions.env`, makes sure that Zephyr and SDK are in the shared volume, and registers them. First start on a machine takes ~10 minutes; every start after that, in any project, takes seconds and touches no network.

The actual downloading lives in **`fetch-zephyr.sh`**, which `setup-sdks.sh` calls when something is missing. You can also call it by hand to add another version alongside:

```bash
bash .devcontainer/fetch-zephyr.sh v4.3.0 0.17.4   # install
use-vanilla v4.3.0 0.17.4                          # switch this shell to it
```

What `setup-sdks.sh` does, in order:

1. **Desync guard.** Compares `ZEPHYR_BASE` / `ZEPHYR_SDK_INSTALL_DIR` (from `devcontainer.json`) against what `versions.env` implies, and exits with both paths printed if they disagree. Two places holding a version is fine; two places quietly disagreeing means provisioning one version and building against another.
2. **`flock`** on `/workdir/zephyr-sdks/.lock`. Two projects' containers can start at once, and two concurrent `west update`s into one directory produce a corrupt workspace.
3. **Adoption.** A volume provisioned before `.complete` sentinels existed gets one stamped, if the install genuinely looks finished — so upgrading does not trigger a pointless refetch. The checks are narrow on purpose: a hollow SDK has neither `setup.sh` nor `cmake/Zephyr-sdkConfig.cmake`, so it is *not* adopted and gets refetched.
4. **Populate**, if a `.complete` sentinel is missing — or if a toolchain named in `ZSDK_TOOLCHAINS` is absent. That second case matters because the store is shared: another project may have provisioned it with a shorter list, and `setup.sh -t` is incremental, so the volume accumulates the union of what every project needs.
5. **Verify**, including that `cmake/Zephyr-sdkConfig.cmake` exists.
6. **Re-register on every start** — `setup.sh -h -c` plus `register-sdks.sh`, then assert the registry entry actually landed.

`versions.env` is the only file to edit per project:

```bash
ZEPHYR_VERSION=v4.2.2
ZSDK_VERSION=0.17.0
ZSDK_TOOLCHAINS="arm-zephyr-eabi x86_64-zephyr-elf xtensa-espressif_esp32_zephyr-elf xtensa-espressif_esp32s3_zephyr-elf"
```

Flags worth knowing:

- `_minimal.tar.xz`: SDK variant with no toolchains preinstalled — you add only what you need via `-t`. The full SDK is roughly 13 GB; minimal plus four targets is a fraction of that.
- `setup.sh -h`: install host tools (qemu, openocd, and friends). This is where `qemu-system-*` comes from — it is not an apt package.
- `setup.sh -c`: register the SDK as a CMake package, so `find_package(Zephyr)` finds it without env vars.
- `setup.sh -t <triple>`: install one toolchain. Repeatable and incremental. `arm-zephyr-eabi` covers nRF53 and STM32; `x86_64-zephyr-elf` covers `native_sim/native/64`; the two `xtensa-espressif_*` entries cover ESP32 and ESP32-S3.
- `west init -l <dir>`: local init — treat an *existing* directory as the manifest repo. Without `-l`, west would clone a manifest from a URL instead.
- `west update --narrow`: fetch only the revisions named in the manifest, not every branch and tag.
- `-o=--depth=1`: pass `--depth=1` through to the underlying `git clone`. Shallow, much faster, no history.

To change Zephyr version: edit `versions.env` **and** the two matching paths in `devcontainer.json`, then restart. No `docker volume rm` needed — the new version is installed alongside the old, and both stay usable via `use-vanilla`.

---

## `register-sdks.sh` — why the nRF Connect extension sees the SDKs

The extension does **not** scan an install directory for SDKs. It reads the CMake
*user package registry* under `~/.cmake/packages/`, where each file contains a single
path:

| Registry entry | Path it holds | What the extension shows |
|---|---|---|
| `Zephyr/<md5>` | `<topdir>/zephyr/share/zephyr-package/cmake` | the SDK, in the SDK picker (it walks four levels up to get the west topdir) |
| `Zephyr-sdk/<md5>` | `<zephyr-sdk-x.y.z>/cmake` | a **Zephyr SDK** toolchain |
| `ZephyrUnittest/<md5>` | `<topdir>/zephyr/share/zephyrunittest-package/cmake` | nothing directly — but `find_package(ZephyrUnittest)` needs it, which is what `unit_testing` ztest builds use |

`west zephyr-export` writes the first and third; `zephyr-sdk-x.y.z/setup.sh -c` writes
the second. Both run only at provisioning time — and `~/.cmake` is **not** on a
volume, so it dies with the container while the SDKs themselves survive in
`zephyr-sdks-cache` / `ncs-sdks-cache`. That is the mismatch: SDKs present, picker
empty.

`register-sdks.sh` closes it. It runs from `setup-sdks.sh` (i.e. `postStartCommand`)
on every start, globs `/workdir/*/*/zephyr/share/zephyr-package/cmake` and
`/workdir/*/toolchains/zephyr-sdk-*/cmake`, writes an entry per hit (filename is the
`md5sum` of the path, content is the bare path), and prunes entries whose target no
longer exists. The `/workdir/*/*/` shape picks up both `zephyr-sdks/` and `ncs-sdks/`
and skips nrfutil's `toolchains/`, `downloads/` and `tmp/` for free.

It also globs `/opt/zephyr-sdks/...`, a second store that ships **empty** in the image.
It exists so a future prebuilt or GHCR-published image can bake a default Zephyr + SDK
in without changing any of this. `use-vanilla` searches `/workdir` first, then `/opt`.

**nRF Connect SDK toolchains are the one exception** — those come from the
extension's bundled nrfutil, not from the registry, and it only looks where
`nrf-connect.toolchainManager.installDirectory` points. `devcontainer.json` sets it
to `/workdir/ncs-sdks`. Without that setting an NCS installed by `use-ncs` is present
on disk but never listed. Cross-check from a terminal:

```bash
nrfutil toolchain-manager list --install-dir /workdir/ncs-sdks
```

**Only one Zephyr SDK should list now.** The old `zephyrprojectrtos/zephyr-build` base
image shipped a second SDK at `/opt/toolchains/zephyr-sdk-1.0.1`, which the extension
found on its own and offered alongside the real one. Building from Ubuntu removed it.
If you ever see two, something is installing an SDK outside `versions.env`.

**If the picker is ever empty**, in the container:

```bash
bash .devcontainer/register-sdks.sh
for f in ~/.cmake/packages/*/*; do echo "$f => $(cat $f)"; done
```

then Command Palette → **nRF Connect: Refresh SDKs** and **nRF Connect: Refresh
Toolchains**.

**The registry is not only for the extension.** `cmake/modules/FindZephyr-sdk.cmake`
has two paths: if `ZEPHYR_SDK_INSTALL_DIR` is set it uses that directly, and if it is
not, it falls back to hardcoded prefixes plus this same user package registry. So a
missing registry entry breaks `west build` and `west twister` too, in any context
where the env var did not survive — a VS Code task, a `docker exec`, a twister
subprocess. The failure is opaque:

```
CMake Error at .../FindZephyr-sdk.cmake:80 (find_package):
  Could not find a package configuration file provided by "Zephyr-sdk"
```

The old `zephyr-build` base image baked a registry entry into an image layer, so this
fallback was always populated and quietly covered for a broken volume. Building from
Ubuntu removed that safety net, which is why `setup-sdks.sh` now asserts the entry
exists after registering.

---

## SDK Switching Helpers

Appended to `/root/.bashrc` by `Dockerfile.devel`. A bare `use-vanilla` runs at the bottom, so every interactive shell starts on vanilla Zephyr.

```bash
use-vanilla [version] [sdk_version]   # default: whatever the project is configured for
use-ncs [version]                     # default: v3.3.0 — installs on demand if missing
reset-ncs                             # restore container baseline
zephyr-stores                         # list installed Zephyr trees and SDKs, and where
ncs.py --list                         # list installed NCS versions
ncs.py install v3.3.0                 # install without switching
```

**There are no version literals in `Dockerfile.devel`.** `use-vanilla` derives its defaults from the `ZEPHYR_BASE` / `ZEPHYR_SDK_INSTALL_DIR` that `devcontainer.json` exported, which in turn track `versions.env`. A hardcoded default there would be a third place holding the version, in the one file that must stay identical across projects — copy this `.devcontainer/` into a project on v4.3.0 and every shell would silently snap back.

**A failed switch leaves the environment untouched.** `use-vanilla` resolves and validates the target — including that `cmake/Zephyr-sdkConfig.cmake` exists — *before* calling `reset-ncs`. Bailing out after the reset would leave the shell with no `ZEPHYR_SDK_INSTALL_DIR` at all, which is precisely the state that produces the opaque `Could not find a package configuration file` error above.

What they manipulate:

- `ZEPHYR_BASE` — which Zephyr tree west builds against
- `ZEPHYR_SDK_INSTALL_DIR` — which toolchain
- `PATH` / `LD_LIBRARY_PATH` — prepended for NCS, restored from `_ORIG_*` on reset

`_ORIG_PATH` and `_ORIG_LD_LIBRARY_PATH` are captured once on first shell load. `reset-ncs` restores from them rather than trying to subtract entries, which is why switching back and forth does not accumulate junk in `PATH`.

`ncs.py` does not modify the environment itself — it **prints `export` statements to stdout**, which `use-ncs` wraps in `eval`. A child process cannot change its parent's environment, so this is the standard workaround. It also means every diagnostic line in `ncs.py` must go to stderr (`>&2`), or it would get eval'd as a command.

### The non-interactive shell trap

`.bashrc` is only sourced by **interactive** shells. CI steps, `postCreateCommand`, and `docker exec sh -c` are all non-interactive, so `use-vanilla` never runs and `ZEPHYR_BASE` is unset. Builds fail with "ZEPHYR_BASE not defined" while working fine in your terminal.

Fixes, in order of preference:

```bash
# 1. Point BASH_ENV at the definitions so non-interactive bash sources them
export BASH_ENV=/root/.bashrc

# 2. Or set the variable at container level in devcontainer.json
"containerEnv": { "ZEPHYR_BASE": "/workdir/zephyr-sdks/v4.2.2/zephyr" }

# 3. Or source explicitly in the CI step
bash -c 'source /root/.bashrc && west build ...'
```

---

## USB Passthrough (Windows / WSL2)

Run on the **Windows host** in an admin PowerShell, not inside the container:

```powershell
usbipd list                        # find the BUSID of the probe
usbipd bind --busid 1-4            # one-time per device, persists across reboots
usbipd attach --wsl --busid 1-4    # every time you replug
usbipd detach --busid 1-4          # hand the device back to Windows
```

Verify inside the container:

```bash
ls -l /dev/ttyACM* /dev/ttyUSB*    # console ports
lsusb                              # probe enumeration
nrfutil device list                # Nordic probes specifically
```

- `bind` marks the device shareable; `attach` actually moves it. The attach is lost on unplug and on WSL restart, `bind` is not.
- If `lsusb` shows the device but `/dev/ttyACM*` is missing, the node was not created inside the container — that is the `rslave` propagation issue, not a usbipd one.

---

## `.clangd`

```yaml
CompileFlags:
  CompilationDatabase: twister-out/esp32s3_devkitc_esp32s3_procpu/zephyr/drivers.gpio.button_toggle
```

- Flags come from the CMake-generated `compile_commands.json`, not a hand-written include list. **You must build at least once** or every `<zephyr/...>` include shows as not found.
- `CompilationDatabase` is a *directory* containing `compile_commands.json`, not the file itself.
- `Remove:` strips GCC-only and Xtensa-only flags that clang rejects (`-mlongcalls`, `-fstrict-volatile-bitfields`, and so on). Extend this list whenever a new target introduces a flag clang chokes on.
- Point it at whichever build you are actively editing — switching boards means editing this path.

---

## Dockerfiles (overview)

A three-stage chain, mirroring the structure of
[zephyrproject-rtos/docker-image](https://github.com/zephyrproject-rtos/docker-image)
(`Dockerfile.base` → `Dockerfile.ci` → `Dockerfile.devel`), split by *who needs each piece*:

| File | Image | Contents | Size |
|---|---|---|---|
| `Dockerfile.base` | `zephyr-workshop-base` | `FROM ubuntu:24.04`. Build tooling, 32-bit multilib, Python venv + Zephyr's Python requirements, static-analysis tools | ~2.8 GB |
| `Dockerfile.ci` | `zephyr-workshop-ci` | + nrfutil, J-Link, GitHub Actions runner — everything needed to flash and test on hardware | ~3.9 GB |
| `Dockerfile.devel` | `zephyr-workshop-devel` | + clangd, clang-format, picocom, `.bashrc` helpers | ~4.3 GB |

Each stage takes `ARG BASE_IMAGE` and `build.sh` chains them. `devel` is what runs.

**Why not `FROM zephyrprojectrtos/zephyr-build`?** It is ~32 GB, and ~90% of that is
irrelevant here: the full SDK with all ~15 toolchains, ARM FVPs, Renode, a Rust
toolchain, Hexagon and TriCore cross compilers, babblesim, a VNC stack. Deriving and
deleting does not help — layers are additive, so an `rm` only writes a whiteout and the
bytes stay in the pull. Upstream's smallest published image, `ci-base` (no SDK), is
still ~8.9 GB.

Notable blocks:

- **Multilib** — `gcc-multilib`/`g++-multilib` plus `libsdl2-dev:i386`. `native_sim` is a 32-bit host binary; the first pair provides `/lib32/libasan.so.*` and `/lib32/libubsan.so.*` that `sanitizers_native-sim_ci.yml` asserts, the second lets display/LVGL samples link.
- **Python venv** at `/opt/python-venv`, first on `PATH`. Ubuntu 24.04 marks the system interpreter externally-managed (PEP 668), so this is required, not stylistic. Requirements are pinned to `ARG ZEPHYR_REQ_REF` and fetched from GitHub — one venv serves every Zephyr version, so it does not have to live in the volume.
- **Static analysis** — `sparse`, in `base`, for `west build -- -DZEPHYR_SCA_VARIANT=sparse`. Zephyr 4.2 ships sca implementations for `clang`, `codechecker`, `coverity`, `cpptest`, `eclair`, `gcc`, `polyspace` and `sparse` — **there is no `clang-tidy` variant**, and the `clang` one wants `analyze-build`, which Ubuntu's `clang-tools` does not provide. `clang-tidy` is in `devel` instead, as editor tooling: clangd runs its checks inline, and you can run it by hand over the `compile_commands.json` that `.clangd` already points at. Install the *unversioned* metapackage — `clang-tidy-18` alone gives only the versioned binary name.
- **Debug symbols** — `libc6-dbg` and `libc6-dbg:i386`. Valgrind refuses to start on a binary whose libc has no debuginfo (`Cannot continue -- exiting now`), and `native_sim/native` is 32-bit, so `twister --enable-valgrind` needs the `:i386` one specifically.
- **Nordic Command Line Tools + J-Link** (`ci`) — the fiddly one. Extracts the NCLT bundle, deletes the 32-bit J-Link variants, then symlinks `libjlinkarm.so.7` at the 64-bit library explicitly. Getting this wrong produces an `ELFCLASS32` error at flash time.
- **Build-time asserts** — each stage checks what it just installed and fails the *image build* rather than letting the problem surface on someone's first build or flash. `base` checks the 32-bit sanitizer runtimes, 32-bit SDL, the SCA tools, and west/cmake/pytest; `ci` checks `file -L | grep 'ELF 64-bit'` on the J-Link library plus `nrfutil device --version`.

Conventions worth keeping:

- `FROM ubuntu:24.04` and pinned `ARG`s — not `:latest`. A base that shifts under a hardware CI runner is a debugging session you do not want.
- `SHELL ["/bin/bash", "-o", "pipefail", "-c"]` — without `pipefail`, a failing `wget` in `wget ... | tar` is masked by tar's success and the image builds broken.
- `--no-install-recommends` plus `rm -rf /var/lib/apt/lists/*` in the same `RUN` — keeps the layer small; splitting them into two `RUN`s would shrink nothing.
- SDKs deliberately **not** installed here. They are runtime, in volumes, so an image rebuild does not cost a re-download.
- No version literals in `Dockerfile.devel` — see [SDK Switching Helpers](#sdk-switching-helpers).

Rebuild:

```bash
bash .devcontainer/build.sh                    # all three stages, layer-cached
docker build --no-cache -f .devcontainer/Dockerfile.base -t zephyr-workshop-base:local .devcontainer
```

**Dev Containers: Rebuild Container** recreates the container but does **not** rebuild
these images — `devcontainer.json` names a prebuilt image. Run `build.sh` for that
(`initializeCommand` does it automatically on open).

---

## Troubleshooting

| Symptom | Layer | Fix |
|---|---|---|
| `ZEPHYR_BASE not defined` in CI, fine in terminal | shell | non-interactive shell — set `BASH_ENV` or `containerEnv` |
| `Could not find a package configuration file provided by "Zephyr-sdk"` | volume | SDK directory exists but is hollow, or the CMake registry is empty. `rm -rf /workdir/zephyr-sdks/toolchains/zephyr-sdk-<ver>` then `bash .devcontainer/setup-sdks.sh` |
| `ZEPHYR_BASE does not match versions.env` on start | versions.env | `devcontainer.json`'s `containerEnv` paths and `versions.env` disagree; make them match, then rebuild the container |
| Container will not start, `initializeCommand` failed | host | no `bash` on the host — install Git Bash on Windows, or run `bash .devcontainer/build.sh` manually |
| `ELFCLASS32` / `libjlinkarm.so.7` | Dockerfile.ci | 32-bit J-Link linked; `bash .devcontainer/build.sh` |
| Probe in `lsusb`, no `/dev/ttyACM*` | devcontainer.json | `bind-propagation=rslave` on the `/dev` mount |
| "Cannot connect to J-Link" *after* detection | devcontainer.json | stale USB node after re-enumeration; same `rslave` fix |
| Runner will not start as root | devcontainer.json | `RUNNER_ALLOW_RUNASROOT=1` |
| `<zephyr/...>` not found in editor | `.clangd` | build once; check `CompilationDatabase` path exists |
| Wrong Zephyr version | versions.env | edit `versions.env` **and** `devcontainer.json`, then restart. No `docker volume rm` — versions install side by side |
| Runner re-registers on every rebuild | devcontainer.json | `${localWorkspaceFolderBasename}-actions-runner` volume missing or renamed |
| Xtensa (or any) compiler not found | versions.env | add the triple to `ZSDK_TOOLCHAINS` and restart; `setup-sdks.sh` tops up the shared SDK incrementally |
| Image changes not taking effect after "Rebuild Container" | build.sh | that recreates the container, not the images. Run `bash .devcontainer/build.sh` |
| nRF Connect SDK picker empty, SDKs on disk | `~/.cmake` | `bash .devcontainer/register-sdks.sh`, then **Refresh SDKs** |
| NCS installed by `use-ncs` never listed | devcontainer.json | `nrf-connect.toolchainManager.installDirectory` must be `/workdir/ncs-sdks` |
| Only NCS missing, vanilla Zephyr fine | both of the above | they are separate mechanisms -- registry for SDKs, nrfutil for NCS toolchains |
| Two Zephyr SDKs offered | volume | no longer expected — the stray `/opt/toolchains/zephyr-sdk-1.0.1` came from the old `zephyr-build` base. Run `zephyr-stores` to see what is actually installed and where |