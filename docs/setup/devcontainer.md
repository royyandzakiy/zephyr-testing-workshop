# Devcontainer reference

For someone debugging their own container, or changing what is in it. Not needed to
use this repo: the devcontainer works without reading any of this.

**General Notes**:

- Four layers, each owning a different thing. Almost all "why isn't my change taking effect" confusion comes from editing the wrong layer.
- The Zephyr SDK is **not baked into the image**. It lives in a named Docker volume, provisioned on first start. This keeps the image small and lets it survive image rebuilds.
- The volume is **shared by name across every project** that copies this `.devcontainer/`. Zephyr and the SDK are downloaded once per *machine*, not once per project.
- The images are built ground-up from `ubuntu:24.04`, not derived from `zephyrprojectrtos/zephyr-build` (~32 GB, mostly toolchains and emulators this repo never uses), and are **published to GHCR** - the devcontainer pulls, it does not build.
- This repo is **not** a west manifest repo. There is no `west.yml`; `ZEPHYR_BASE` points outside the workspace into the SDK volume. That is why the env-var shell helpers exist at all. A `west.yml` here would mean west's T2 topology, which clones Zephyr *into each project's* workspace - exactly what the shared volume exists to avoid.

## Where the image comes from

The images live in a separate repo, **[zephyr-devcontainer](https://github.com/royyandzakiy/zephyr-devcontainer)**,
and are published to GHCR. This repo builds nothing - it pulls.

That repo holds the Dockerfiles, the build chain, the publish workflow, the
comparison against upstream `zephyrproject-rtos/docker-image`, and the runtime
scripts. Read it when you need to change the image; read this file when you need
to understand how a container behaves.

```
ghcr.io/royyandzakiy/zephyr-devcontainer-devel:<tag>   what devcontainer.json pulls
ghcr.io/royyandzakiy/zephyr-devcontainer-ci:<tag>      the `container:` in .github/workflows
```

**The runtime scripts ship inside the image** at `/opt/devcontainer/`
(`setup-sdks.sh`, `fetch-zephyr.sh`, `register-sdks.sh`, `ncs.py`). That is why
this repo's `.devcontainer/` holds only two files. Editing one of those scripts
means a change in the other repo, a republish, and a `docker pull` - not a
container restart.

**`containerEnv` is the only configuration.** There is no `versions.env` any
more. `ZEPHYR_BASE` and `ZEPHYR_SDK_INSTALL_DIR` have to be set in
`devcontainer.json` regardless (the VS Code extension host cannot read an env
file), both version numbers are derived from those paths, and `ZSDK_TOOLCHAINS`
and `ZEPHYR_BLOBS` sit alongside them. One source of truth, so the desync guard
that used to live in `setup-sdks.sh` is gone.

Order of truth: the published image defines what exists → `devcontainer.json`
decides how it is launched, mounted and configured → `setup-sdks.sh` fills the
volumes with whatever `containerEnv` names. Missing tool, change the image repo.
Missing device, path or version, change `devcontainer.json`.

## `devcontainer.json`

```json
{
  "name": "Zephyr Development",
  "image": "ghcr.io/royyandzakiy/zephyr-devcontainer-devel:z4.4.2-sdk1.0.1",
  "containerEnv": {
    "RUNNER_ALLOW_RUNASROOT": "1",
    "ZEPHYR_BASE": "/workdir/zephyr-sdks/v4.4.2/zephyr",
    "ZEPHYR_SDK_INSTALL_DIR": "/workdir/zephyr-sdks/toolchains/zephyr-sdk-1.0.1",
    "ZEPHYR_TOOLCHAIN_VARIANT": "zephyr",
    "ZSDK_TOOLCHAINS": "arm-zephyr-eabi x86_64-zephyr-elf ...",
    "ZEPHYR_BLOBS": "hal_espressif"
  },
  "containerUser": "root",
  "remoteUser": "root",
  "updateRemoteUserUID": false,
  "runArgs": ["--privileged"],
  "mounts": [ ... ],
  "workspaceFolder": "/workspaces/${localWorkspaceFolderBasename}",
  "postStartCommand": "/opt/devcontainer/setup-sdks.sh"
}
```

### Two devcontainer configs

| Config | For |
|---|---|
| `.devcontainer/` | everyone - pulls the published image |
| `.devcontainer/macos/` | same image, `--platform=linux/amd64` for Apple Silicon |

Neither has an `initializeCommand`, so **Docker is the only host requirement** - no local build, and nothing that needs `bash` on the host.

There is no build-from-source variant here any more. To change the image, work in
[zephyr-devcontainer](https://github.com/royyandzakiy/zephyr-devcontainer), which has its own devcontainer for that.

The tag is the Zephyr/SDK pair (`z4.4.2-sdk1.0.1`). Bumping the version in the
image repo publishes a *new* tag rather than moving this one, so nothing shifts
under an attendee mid-workshop. Keep the tag in step across both files here.

### Identity and privileges

- `containerUser`: user the container process runs as. Root here, because flashing needs raw USB access and the Actions runner writes to `/actions-runner`.
- `remoteUser`: user the editor server and its terminals run as. Usually matches `containerUser`.
- `updateRemoteUserUID`: normally the editor rewrites the container user's UID to match your host UID, so bind-mounted files are not root-owned. **Must be `false` when the user is already root** - otherwise the rewrite fails or mangles ownership.
- `containerEnv`: env vars set at container level, visible to every process including non-interactive shells. Unlike `.bashrc` exports, these survive into CI steps.
  - `RUNNER_ALLOW_RUNASROOT=1`: the GitHub Actions runner refuses to start as root without it. Only needed because of the self-hosted runner.
- `runArgs`: raw arguments passed straight to `docker run`.
  - `--privileged`: full device access. Required for USB probe passthrough (J-Link, ST-LINK, ESP32 native USB). Blunt instrument - `--device-cgroup-rule` is narrower but breaks on re-enumeration.

### Mounts

```json
"mounts": [
  "source=zephyr-sdks,target=/workdir/zephyr-sdks,type=volume",
  "source=ncs-sdks,target=/workdir/ncs-sdks,type=volume",
  "source=${localWorkspaceFolderBasename}-actions-runner,target=/actions-runner,type=volume",
  "source=/dev,target=/dev,type=bind,bind-propagation=rslave"
]
```

- `type=volume`: Docker-managed named volume. Survives container deletion and image rebuild. Use for anything expensive to re-download.
  - `zephyr-sdks` - vanilla Zephyr source + Zephyr SDK toolchain
  - `ncs-sdks` - NCS toolchains, managed by `nrfutil toolchain-manager`
  - `${localWorkspaceFolderBasename}-actions-runner` - runner registration and credentials, so you do not re-register on every rebuild
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
| `postStartCommand` | **every start**, including restarts | `setup-sdks.sh` - must be idempotent |
| `postAttachCommand` | every time an editor attaches | editor-only fixups |

The default config uses only `postStartCommand`, for `setup-sdks.sh`, which therefore has to be idempotent because it runs on every single start.

Idempotent here means `.complete` sentinel files, **not** `if [ ! -d ... ]` guards. A directory-existence check treats "the download started" as "the install finished": a `tar` that creates the directory and then dies leaves a hollow SDK that every later start reports as ready. That bug is exactly how this repo ended up with an SDK directory containing no SDK. Sentinels are written only after a step fully succeeds.

### Editor config

- `customizations.vscode.extensions`: auto-installed on attach. Purely editor-side, ignored by everything else.
  - `nordic-semiconductor.nrf-connect` - board and DTS tooling
- `customizations.vscode.settings`: `nrf-connect.toolchainManager.installDirectory` points the extension's bundled nrfutil at `/workdir/ncs-sdks`. See [`register-sdks.sh`](#register-sdkssh--why-the-nrf-connect-extension-sees-the-sdks).
  - `thecreativedodo.usbip-connect` - attaches WSL2 USB devices from inside the editor
  - `ms-vscode.cpptools` is installed, but `.clangd` is the config actually in use. Pick one - running IntelliSense and clangd together produces duplicate, contradictory diagnostics.

---

## Named Volumes

```bash
docker volume ls                                  # list
docker volume inspect zephyr-sdks           # mountpoint + metadata
docker volume rm zephyr-sdks                # force full re-provision on next start
docker volume prune                               # remove all unreferenced volumes
```

The container must be stopped before `rm` succeeds. After removing `zephyr-sdks`, the next start re-downloads Zephyr and the SDK - roughly 10 minutes, so not something to do casually. Note this also affects **every other project** on the machine that shares the volume.

To see what is actually installed and which store it came from, run `zephyr-stores` inside the container.

Rebuild paths (Command Palette):

- **Dev Containers: Rebuild Container** - recreate container, keep image cache, keep volumes
- **Dev Containers: Rebuild Without Cache** - full Dockerfile re-run, keep volumes
- Volumes are only removed by `docker volume rm`, never by a rebuild.

---

## `setup-sdks.sh`

Ships in the image at `/opt/devcontainer/setup-sdks.sh` and runs on every container start. Reads its configuration from `containerEnv`, makes sure that Zephyr and the SDK are in the shared volume, and registers them. First start on a machine takes ~10 minutes; every start after that, in any project, takes seconds and touches no network.

The actual downloading lives in **`fetch-zephyr.sh`**, which `setup-sdks.sh` calls when something is missing. You can also call it by hand to add another version alongside:

```bash
/opt/devcontainer/fetch-zephyr.sh v4.3.0 0.17.4   # install
use-vanilla v4.3.0 0.17.4                          # switch this shell to it
```

What `setup-sdks.sh` does, in order:

1. **Config check.** Requires `ZEPHYR_BASE`, `ZEPHYR_SDK_INSTALL_DIR` and `ZSDK_TOOLCHAINS`, and says which one is missing and where it belongs. The two version numbers are derived from the two paths, so there is a single source of truth and nothing to cross-check. (An earlier design kept them in a `versions.env` too, which needed a guard against the two disagreeing.)
2. **`flock`** on `/workdir/zephyr-sdks/.lock`. Two projects' containers can start at once, and two concurrent `west update`s into one directory produce a corrupt workspace.
3. **Adoption.** A volume provisioned before `.complete` sentinels existed gets one stamped, if the install genuinely looks finished - so upgrading does not trigger a pointless refetch. The checks are narrow on purpose: a hollow SDK has neither `setup.sh` nor `cmake/Zephyr-sdkConfig.cmake`, so it is *not* adopted and gets refetched.
4. **Populate**, if a `.complete` sentinel is missing - or if a toolchain named in `ZSDK_TOOLCHAINS` is absent. That second case matters because the store is shared: another project may have provisioned it with a shorter list, and `setup.sh -t` is incremental, so the volume accumulates the union of what every project needs.
5. **Verify**, including that `cmake/Zephyr-sdkConfig.cmake` exists.
6. **Re-register on every start** - `setup.sh -h -c` plus `register-sdks.sh`, then assert the registry entry actually landed.

`containerEnv` in `devcontainer.json` is the only place this is configured:

```bash
ZEPHYR_VERSION=v4.4.2
ZSDK_VERSION=1.0.1
ZSDK_TOOLCHAINS="arm-zephyr-eabi x86_64-zephyr-elf xtensa-espressif_esp32_zephyr-elf xtensa-espressif_esp32s3_zephyr-elf"
```

Flags worth knowing:

- `_minimal.tar.xz`: SDK variant with no toolchains preinstalled - you add only what you need via `-t`. The full SDK is roughly 13 GB; minimal plus four targets is a fraction of that.
- `setup.sh -h`: install host tools (qemu, openocd, and friends). This is where `qemu-system-*` comes from - it is not an apt package.
- `setup.sh -c`: register the SDK as a CMake package, so `find_package(Zephyr)` finds it without env vars.
- `setup.sh -t <triple>`: install one toolchain. Repeatable and incremental. `arm-zephyr-eabi` covers nRF53 and STM32; `x86_64-zephyr-elf` covers `native_sim/native/64`; the two `xtensa-espressif_*` entries cover ESP32 and ESP32-S3.
- `west init -l <dir>`: local init - treat an *existing* directory as the manifest repo. Without `-l`, west would clone a manifest from a URL instead.
- `west update --narrow`: fetch only the revisions named in the manifest, not every branch and tag.
- `-o=--depth=1`: pass `--depth=1` through to the underlying `git clone`. Shallow, much faster, no history.

To change Zephyr version: edit the two paths in `devcontainer.json`, then restart. No `docker volume rm` needed - the new version is installed alongside the old, and both stay usable via `use-vanilla`.

---

## `register-sdks.sh` - why the nRF Connect extension sees the SDKs

The extension does **not** scan an install directory for SDKs. It reads the CMake
*user package registry* under `~/.cmake/packages/`, where each file contains a single
path:

| Registry entry | Path it holds | What the extension shows |
|---|---|---|
| `Zephyr/<md5>` | `<topdir>/zephyr/share/zephyr-package/cmake` | the SDK, in the SDK picker (it walks four levels up to get the west topdir) |
| `Zephyr-sdk/<md5>` | `<zephyr-sdk-x.y.z>/cmake` | a **Zephyr SDK** toolchain |
| `ZephyrUnittest/<md5>` | `<topdir>/zephyr/share/zephyrunittest-package/cmake` | nothing directly - but `find_package(ZephyrUnittest)` needs it, which is what `unit_testing` ztest builds use |

`west zephyr-export` writes the first and third; `zephyr-sdk-x.y.z/setup.sh -c` writes
the second. Both run only at provisioning time - and `~/.cmake` is **not** on a
volume, so it dies with the container while the SDKs themselves survive in
`zephyr-sdks` / `ncs-sdks`. That is the mismatch: SDKs present, picker
empty.

`register-sdks.sh` closes it. It runs from `setup-sdks.sh` (i.e. `postStartCommand`)
on every start, globs `/workdir/*/*/zephyr/share/zephyr-package/cmake` and
`/workdir/*/toolchains/zephyr-sdk-*/cmake`, writes an entry per hit (filename is the
`md5sum` of the path, content is the bare path), and prunes entries whose target no
longer exists. The `/workdir/*/*/` shape picks up both `zephyr-sdks/` and `ncs-sdks/`
and skips nrfutil's `toolchains/`, `downloads/` and `tmp/` for free.

It also globs `/opt/zephyr-sdks/...`, a second store that ships **empty** in the image.
The `ci` image bakes a Zephyr + SDK pair into it for GitHub Actions; in the `devel`
image it ships empty, because development uses the `/workdir` volume. `use-vanilla` searches `/workdir` first, then `/opt`.

**nRF Connect SDK toolchains are the one exception** - those come from the
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
If you ever see two, something is installing an SDK outside `ZSDK_TOOLCHAINS`.

**If the picker is ever empty**, in the container:

```bash
/opt/devcontainer/register-sdks.sh
for f in ~/.cmake/packages/*/*; do echo "$f => $(cat $f)"; done
```

then Command Palette → **nRF Connect: Refresh SDKs** and **nRF Connect: Refresh
Toolchains**.

**The registry is not only for the extension.** `cmake/modules/FindZephyr-sdk.cmake`
has two paths: if `ZEPHYR_SDK_INSTALL_DIR` is set it uses that directly, and if it is
not, it falls back to hardcoded prefixes plus this same user package registry. So a
missing registry entry breaks `west build` and `west twister` too, in any context
where the env var did not survive - a VS Code task, a `docker exec`, a twister
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

Appended to `/root/.bashrc` by the image. A bare `use-vanilla` runs at the bottom, so every interactive shell starts on vanilla Zephyr.

```bash
use-vanilla [version] [sdk_version]   # default: whatever the project is configured for
use-ncs [version]                     # default: v3.3.0 - installs on demand if missing
reset-ncs                             # restore container baseline
zephyr-stores                         # list installed Zephyr trees and SDKs, and where
ncs.py --list                         # list installed NCS versions
ncs.py install v3.3.0                 # install without switching
```

**There are no version literals in the image.** `use-vanilla` derives its defaults from the `ZEPHYR_BASE` / `ZEPHYR_SDK_INSTALL_DIR` that `devcontainer.json` exported. A hardcoded default would override every consuming project at once - a project pinned to v4.3.0 would find every shell silently snapping back.

**A failed switch leaves the environment untouched.** `use-vanilla` resolves and validates the target - including that `cmake/Zephyr-sdkConfig.cmake` exists - *before* calling `reset-ncs`. Bailing out after the reset would leave the shell with no `ZEPHYR_SDK_INSTALL_DIR` at all, which is precisely the state that produces the opaque `Could not find a package configuration file` error above.

What they manipulate:

- `ZEPHYR_BASE` - which Zephyr tree west builds against
- `ZEPHYR_SDK_INSTALL_DIR` - which toolchain
- `PATH` / `LD_LIBRARY_PATH` - prepended for NCS, restored from `_ORIG_*` on reset

`_ORIG_PATH` and `_ORIG_LD_LIBRARY_PATH` are captured once on first shell load. `reset-ncs` restores from them rather than trying to subtract entries, which is why switching back and forth does not accumulate junk in `PATH`.

`ncs.py` does not modify the environment itself - it **prints `export` statements to stdout**, which `use-ncs` wraps in `eval`. A child process cannot change its parent's environment, so this is the standard workaround. It also means every diagnostic line in `ncs.py` must go to stderr (`>&2`), or it would get eval'd as a command.

### The non-interactive shell trap

`.bashrc` is only sourced by **interactive** shells. CI steps, `postCreateCommand`, and `docker exec sh -c` are all non-interactive, so `use-vanilla` never runs and `ZEPHYR_BASE` is unset. Builds fail with "ZEPHYR_BASE not defined" while working fine in your terminal.

Fixes, in order of preference:

```bash
# 1. Point BASH_ENV at the definitions so non-interactive bash sources them
export BASH_ENV=/root/.bashrc

# 2. Or set the variable at container level in devcontainer.json
"containerEnv": { "ZEPHYR_BASE": "/workdir/zephyr-sdks/v4.4.2/zephyr" }

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
- If `lsusb` shows the device but `/dev/ttyACM*` is missing, the node was not created inside the container - that is the `rslave` propagation issue, not a usbipd one.

---

## `.clangd`

```yaml
CompileFlags:
  # CompilationDatabase: .
```

- Flags come from the CMake-generated `compile_commands.json`, not a hand-written include list. **You must build at least once** or every `<zephyr/...>` include shows as not found.
- `CompilationDatabase` is commented out on purpose. clangd walks up from the source
  file and finds `apps/<app>/build/compile_commands.json` by itself, so building the
  app you are editing is enough and there is no path to keep in step with your board.
- Setting it explicitly is still an option if you want to pin one build. It names a
  *directory* containing `compile_commands.json`, not the file.
- `Remove:` strips GCC-only and Xtensa-only flags that clang rejects (`-mlongcalls`, `-fstrict-volatile-bitfields`, and so on). Extend this list whenever a new target introduces a flag clang chokes on.

---

## Troubleshooting

| Symptom | Layer | Fix |
|---|---|---|
| `ZEPHYR_BASE not defined` in CI, fine in terminal | shell | non-interactive shell - set `BASH_ENV` or `containerEnv` |
| `Could not find a package configuration file provided by "Zephyr-sdk"` | volume | SDK directory exists but is hollow, or the CMake registry is empty. `rm -rf /workdir/zephyr-sdks/toolchains/zephyr-sdk-<ver>` then `/opt/devcontainer/setup-sdks.sh` |
| `ZEPHYR_BASE is not set` / `ZSDK_TOOLCHAINS is not set` on start | devcontainer.json | the variable is missing from `containerEnv`; add it and rebuild the container |
| Container will not start | host | check Docker is running. Neither config here uses `initializeCommand`, so a failure at this stage is Docker itself or the image pull |
| `ELFCLASS32` / `libjlinkarm.so.7` | image | 32-bit J-Link linked; fix and republish in the image repo |
| Probe in `lsusb`, no `/dev/ttyACM*` | devcontainer.json | `bind-propagation=rslave` on the `/dev` mount |
| "Cannot connect to J-Link" *after* detection | devcontainer.json | stale USB node after re-enumeration; same `rslave` fix |
| Runner will not start as root | devcontainer.json | `RUNNER_ALLOW_RUNASROOT=1` |
| `<zephyr/...>` not found in editor | `.clangd` | build that app once, so `apps/<app>/build/compile_commands.json` exists |
| Wrong Zephyr version | devcontainer.json | edit the two paths in `containerEnv`, then restart. No `docker volume rm` - versions install side by side |
| Runner re-registers on every rebuild | devcontainer.json | `${localWorkspaceFolderBasename}-actions-runner` volume missing or renamed |
| Xtensa (or any) compiler not found | devcontainer.json | add the triple to `ZSDK_TOOLCHAINS` in `containerEnv` and restart; `setup-sdks.sh` tops up the shared SDK incrementally |
| Image changes not taking effect after "Rebuild Container" | registry | that recreates the container, not the image. `docker pull` the tag first |
| nRF Connect SDK picker empty, SDKs on disk | `~/.cmake` | `/opt/devcontainer/register-sdks.sh`, then **Refresh SDKs** |
| NCS installed by `use-ncs` never listed | devcontainer.json | `nrf-connect.toolchainManager.installDirectory` must be `/workdir/ncs-sdks` |
| Only NCS missing, vanilla Zephyr fine | both of the above | they are separate mechanisms -- registry for SDKs, nrfutil for NCS toolchains |
| Two Zephyr SDKs offered | volume | no longer expected - the stray `/opt/toolchains/zephyr-sdk-1.0.1` came from the old `zephyr-build` base. Run `zephyr-stores` to see what is actually installed and where |