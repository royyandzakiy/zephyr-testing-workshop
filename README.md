# Practical Embedded Automated Testing for Zephyr

Workshop repository. Everything you build today lives under [`apps/`](apps/).

**The thesis:** the seam between test and hardware belongs in the devicetree, not in your code. By the end of the day the same test file runs against an emulated GPIO controller on your laptop and against a physical board in CI, and the application source is identical in both cases.

---

## Before the workshop

Do this **3–4 days ahead**, not the morning of. The CI session depends on step 1.

1. **Fork this repository** to your own GitHub account. Session 4 pushes to your fork and watches your own Actions run.
2. **Install Docker** — Docker Desktop (Windows/macOS) or Docker Engine (Linux). Confirm `docker run hello-world` works.
3. **Clone your fork** and open it in VS Code. Accept the *"Reopen in Container"* prompt and let the devcontainer build. First build pulls a large image — leave it running.
4. **Verify** inside the container:

   ```bash
   cd apps/00-hello
   west build -b native_sim/native -p
   ./build/zephyr/zephyr.exe
   ```

   You should see a hello line and the board name.

5. **Run the test suite once** — the step most likely to fail, so don't skip it:

   ```bash
   west twister -T apps/ -p native_sim
   ```

Optional, if you have hardware: build and flash to a board you own, and register a local `actions-runner`. Neither is required — nobody is gated on owning a board.

**Stuck?** Open an issue on this repo before the day, or join the 30-minute setup window at the start. We won't debug Docker during teaching time.

Full setup notes: [`docs/PRE-general-guide.md`](docs/PRE-general-guide.md) · [`docs/PRE-build-flash-monitor.md`](docs/PRE-build-flash-monitor.md)

---

## How this repo works

Each folder under `apps/` is a **complete, standalone Zephyr application** and a milestone in the day.

> **App N+1 is the solved state of app N.**

That's the important part. If you fall behind, don't panic and don't try to catch up — `cd` into the next folder and you're back with the room. Nothing carries between folders except understanding.

It also means the diff *is* the lesson:

```bash
diff -r apps/02-ztest apps/03-emul
```

One new directory. Zero changed source files. That's the whole of Session 2.

---

## The apps

| App | Session | What it is |
|---|---|---|
| [`00-hello`](apps/00-hello) | Pre-work · S1 | Bare `printk`. No devicetree, nothing to bind. Your setup check. |
| [`01-blinky`](apps/01-blinky) | S1 | Button toggles an LED. One flat `main.c` — deliberately the code you'd inherit. |
| [`02-ztest`](apps/02-ztest) | S2 | Same behaviour, logic cut out behind a seam. First `ztest` suite. |
| [`03-emul`](apps/03-emul) | S2 | `gpio_emul` drives a fake button. A **test-only overlay** reroutes the alias. |
| [`04-shell-pytest`](apps/04-shell-pytest) | S3 | Shell command as a test backdoor. `pytest` asserts from outside the device. |
| [`05-ci`](apps/05-ci) | S4 | Byte-identical to `04`. Frozen and known-green so CI is the only variable. |

Inside an app:

```
apps/03-emul/
├── CMakeLists.txt
├── prj.conf                 application config
├── app.overlay              devicetree defaults, all boards
├── boards/                  per-board overlays and configs
│   ├── native_sim_native.overlay
│   ├── nrf5340dk_nrf5340_cpuapp.overlay
│   └── ...
├── src/                     application source
├── tests/                   suites for THIS app
│   ├── unit/                ztest, native_sim only
│   └── emul/                ztest + gpio_emul, with its own app.overlay
└── README.md                what changed vs the previous app, and why
```

**Tests live with the app they test.** Twister recurses, so `-T apps/` finds every suite in the repo without a registry to maintain. Each `tests/*/` subfolder is itself a small Zephyr app with its own `prj.conf` and `app.overlay` — that overlay is where the emulation gets swapped in.

---

## Commands

Run everything from inside the devcontainer. Replace the app path as you move through the day.

**Build and run natively**

```bash
cd apps/01-blinky
west build -b native_sim/native -p
./build/zephyr/zephyr.exe
```

**Build for a board and flash**

```bash
west build -b nrf5340dk/nrf5340/cpuapp -p
west flash
```

**Run one app's tests on `native_sim`**

```bash
west twister -T apps/03-emul -p native_sim
```

**Run everything** — what CI does

```bash
west twister -T apps/ -p native_sim
```

**Run against real hardware**

```bash
west twister -T apps/04-shell-pytest \
  --device-testing --hardware-map hardware-map.yaml
```

Edit [`hardware-map.yaml`](hardware-map.yaml) with your own probe serial and serial port first. On ESP32-S3 add `--flash-before`, or the harness holds a stale descriptor after the USB peripheral re-enumerates.

**When something is red:** the answer is in `twister-out/`. Look at `handler.log` for what the device actually said, and `build.log` for what didn't compile. Learning your way around that directory is most of debugging a failed CI run.

---

## The day

| | Session | Focus |
|---|---|---|
| 0:30 | 1 | `native_sim`, build system, devicetree |
| 1:25 | 2 | ztest, emulation, test-only overlays |
| *2:30* | | *break* |
| 3:00 | 3 | Twister and pytest |
| 4:05 | 4 | CI and self-hosted runners |

Two hard gates: everyone's emulated test green before the break, everyone's pytest suite green before Session 4.

Finished a block early? → [`docs/EXERCISE.md`](docs/EXERCISE.md). Graded **★** to **★★★**, all self-serve, no board needed. The answers are in the Zephyr docs rather than in this repo — finding them is the exercise.

---

## Reference

| | |
|---|---|
| [`docs/PYTEST_GUIDE.md`](docs/PYTEST_GUIDE.md) | `twister_harness`, the `dut` and `Shell` fixtures |
| [`docs/NOTES-testing.md`](docs/NOTES-testing.md) | ztest, Twister, `testcase.yaml` |
| [`docs/NOTES-native-sim.md`](docs/NOTES-native-sim.md) | Running and debugging the native binary |
| [`docs/NOTES-ci-self-hosted.md`](docs/NOTES-ci-self-hosted.md) | Runner registration, USB passthrough, `nrfutil` |
| [`docs/NOTES-devcontainer.md`](docs/NOTES-devcontainer.md) | Container internals, SDK layout |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | Working on this repo itself |

Zephyr **v4.2.2**, SDK **0.17.0**. Boards with overlays in-tree: `native_sim`, `nrf5340dk`, `esp32_devkitc`, `esp32s3_devkitc`, `nucleo_g474re`, `qemu_cortex_m3`.

`dump/` is scratch material kept for reference. Ignore it.

---

## After the workshop

Take a module from your own codebase — one you'd normally test by flashing and watching. Find which devicetree nodes it binds to, picture what an emulated version looks like, and write down what the first test would assert.

You don't have to build it. Just find the seam. The interesting cases are the ones where you can't.