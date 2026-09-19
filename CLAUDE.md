# CLAUDE.md

Workshop repository for *Practical Embedded Automated Testing for Zephyr*. Zephyr
**v4.4.2**, SDK **1.0.1**. Each folder under `apps/` is a complete, standalone Zephyr
application plus its own tests.

## The one rule that matters

**Never ship Zephyr code you have not built.** The host can verify pure C logic with
`gcc`, and that is worth doing first, but it tells you nothing about Kconfig,
devicetree, ztest ordering or the twister harness. Those only show up in a real build.

Work happens on Windows; the toolchain lives in a Linux devcontainer. Find it and run
there:

```bash
docker ps --format '{{.ID}}\t{{.Image}}' | grep zephyr-devcontainer
```

```bash
docker exec <id> bash -lc 'cd /workspaces/zephyr-testing-workshop && west twister -T apps/ -p native_sim -O /tmp/tw --clobber-output'
```

Always pass `-O /tmp/<name> --clobber-output`. Twister rotates `twister-out/` into
`twister-out.1` on every run, and that rotation fails on the Windows bind mount with
`OSError: [Errno 39] Directory not empty`.

## Skills

| Skill | Use it when |
|---|---|
| `zephyr-build-run` | building, flashing, monitoring, running twister, or working out why a run went red |
| `zephyr-ztest` | writing on-device tests in C: unit, integration, or emulated-driver |
| `zephyr-pytest` | writing tests that run off the device and talk to it, usually end to end |
| `workshop-app-docs` | writing or refreshing any `README.md` or `EXERCISE.md` under `apps/` |

Pick between the two test skills by scope, not by habit. `zephyr-ztest` covers
everything that runs *on* the device. `zephyr-pytest` covers everything that drives
the device *from outside*. The scoping question is the first section of both.

## Layout

```
apps/NN-name/
├── CMakeLists.txt, prj.conf      the application
├── boards/                       per-board .overlay and .conf, auto-discovered by name
├── src/                          application source
├── tests/<suite>/                each one is its own Zephyr app
│   ├── CMakeLists.txt, prj.conf
│   ├── app.overlay               test-only devicetree, overrides the app's
│   ├── testcase.yaml             scenario names, platform_allow, harness
│   └── src/ or pytest/
├── README.md, EXERCISE.md
└── NOTES.md                      board commands and known findings, where present
```

Twister recurses looking for `testcase.yaml`, so tests live next to the app they test
and there is no registry to maintain.

Scenario names follow `appNN.<domain>.<kind>`, for example `app07.feeder.ztest`.

## Apps

Apps 00 to 06 are one sequence about the seam between test and hardware. Apps 07 to 09
are a second group about what you write once you have found the seam.

| | |
|---|---|
| `00-hello` | printk only, no devicetree. The setup check. |
| `01-blinky` | button and LED through devicetree aliases. No tests. |
| `02-ztest` | logic pulled behind a seam, first ztest suite |
| `03-emul-gpio` | `gpio_emul` plus a test-only overlay that reroutes `sw0` and `led0` |
| `04-shell-pytest` | shell command as a test backdoor, pytest asserts from outside |
| `05-pytest-advanced` | fixtures, parametrize, markers, plus the same suite with no twister |
| `06-sensor` | I2C, the real Bosch BME280 driver, and an emulated chip written here |
| `07-unit-conventions` | ztest conventions over a pond feeder schedule |
| `08-fff-mocks` | FFF. Fake your own port functions, not the chip. |
| `09-gtest-gmock` | GoogleTest and GoogleMock compiled into a Zephyr image |

## Reference docs

| | |
|---|---|
| [`docs/NOTES-build-flash.md`](docs/NOTES-build-flash.md) | the authoritative per-board command reference. Read it before inventing a flash command. |
| [`docs/NOTES-testing.md`](docs/NOTES-testing.md) | on-target and off-target runs for the same suite, with the reason for each flag |
| [`docs/NOTES-native-sim.md`](docs/NOTES-native-sim.md) | PTY versus stdin/stdout console modes |
| [`docs/PYTEST_GUIDE.md`](docs/PYTEST_GUIDE.md) | `twister_harness`, the `dut` and `Shell` fixtures |
| [`docs/NOTES-ci-self-hosted.md`](docs/NOTES-ci-self-hosted.md) | runner registration, USB passthrough |

## House style

No em dashes anywhere, in code comments, docs or commit messages. Use a comma, a
spaced hyphen, parentheses, or two sentences.

Docs under `apps/` are attendee-facing and have their own rules, which live in the
`workshop-app-docs` skill. Load it before touching any `README.md` or `EXERCISE.md`
there.

Code comments explain *why*, especially where a line exists to avoid a specific bug.
`apps/07-unit-conventions/src/feeder.c` is the model: the comment says what the
tempting wrong version does and what it would cost.
