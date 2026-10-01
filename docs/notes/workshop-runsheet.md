> Working notes. My run sheet for the live day, to keep open on the second screen.
> Not written for a reader. Built from [`PI.md`](PI.md), but the repo is current where
> the two disagree. [`WORKSHOP.md`](WORKSHOP.md) is the older brainstorm.

# Run sheet

Online, 1 day, **20:00 to 01:00 WIB**, ~20 people, solo.
ALL = everyone does it. DEMO = I do it, they watch. Everything runs on native_sim unless
marked board.

All twister commands in chat get `-O /tmp/<name> --clobber-output` (the Windows bind
mount breaks `twister-out/` rotation).

## Timetable

| WIB | Min | Block | Mode | App or file | Finish line |
|---|---|---|---|---|---|
| 20:00 | 15 | Setup rescue | ALL | pre-work | 00 runs, 02 green, Actions green |
| 20:15 | 15 | Networking, intro questions | talk | | |
| **20:30** | | **Part 1** | | | |
| 20:30 | 20 | The demo | DEMO, board | `05-shell-pytest` | |
| 20:50 | 15 | Why firmware resists testing | talk | | |
| 21:05 | 30 | Host sim and ztest | ALL | `02-ztest`, ex1 | ✅ ex1 green |
| 21:35 | 35 | GPIO emulation | ALL | `03-emul-gpio/tests/emul` | ✅ emul suite green |
| 22:10 | 20 | Test-only overlays | ALL | ex2 | ✅ ex2 green |
| 22:30 | 30 | **Break** | | | |
| **23:00** | | **Part 2** | | | |
| 23:00 | 25 | Twister | DEMO, follow along | testcase.yaml in 02, 05 | |
| 23:25 | 10 | Designing tests | talk | `07` main.c | |
| 23:35 | 35 | pytest e2e | ALL | `04`, `05`, ex3 | ✅ ex3 green |
| 00:10 | 20 | CI | ALL | their template repo | ✅ red run seen, artifact downloaded |
| 00:30 | 5 | On-target CI | DEMO, board | `test-hardware.yml` | |
| 00:35 | 15 | Agent skills | DEMO | ex2b, skills | |
| 00:50 | 10 | Legacy project, Q&A | talk | checklist below | |
| 01:00 | | End | | | |

Zero slack in either part. Check the clock at every finish line.

**Cut order when late**, first to go at the top:
1. Renode bonus demo (only happens if ahead anyway)
2. Twister: matrix and filtering detail, keep testcase.yaml and twister-out
3. Designing tests down to 5 min
4. On-target CI down to 2 min, screenshots in `docs/imgs/`
5. `05` walkthrough in the pytest block, go straight to ex3
6. twister-triage in the skills block

Never cut: any ALL block, the skills block (it is in the title), the opening demo.

## Solo rules

- Rhythm per ALL block: explain and demo, they do it, debrief. Move on when the timebox
  ends, not when everyone is done.
- At each finish line: ✅ in chat, or ❌ plus the pasted error. Silence tells me nothing.
- Stuck for two attempts or 5 min: paste the error in chat, copy the `solution/` file,
  keep following. Catch up in the break.
- Scan chat for the same error from several people. Fix that on screen once. Do not
  debug individual screens during a block.
- No board debugging for attendees during the session. Boards are optional (PI FAQ 2).

---

## 20:00 Setup rescue (15)

The first 15 min are only for people whose pre-work failed. Everyone else chats.

Checks, in order:
1. `cd apps/00-hello && west build -b native_sim/native -p && ./build/zephyr/zephyr.exe`
2. `west twister -T apps/02-ztest -p native_sim -O /tmp/tw02 --clobber-output`
3. Actions tab on their own template repo: Tests (native_sim) and Sanitizers green

Anyone still broken at 20:15: they watch Part 1, fix it in the break.
Common ones: [Known errors](#known-errors).

## 20:15 Networking (15)

- What industry, what products (if allowed)
- Any testing experience, or none
- What they most want out of tonight. Remember the answers for the Q&A.

## 20:30 The demo (20, DEMO, board)

Goal: the same pytest file passes on the board and on native_sim.

Camera on the DK. Terminal font big.

1. App image on the board, physical button, LED1 toggles, log on the console.
   ```
   west build -b nrf5340dk/nrf5340/cpuapp apps/05-shell-pytest -d /tmp/demo-app -p
   west flash -d /tmp/demo-app
   ```
2. Test image on the board through Twister and the hardware map. LED1 blinks twice on
   camera while pytest asserts.
   ```
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle --device-testing --flash-before --hardware-map apps/05-shell-pytest/hardware-map.yaml -O /tmp/demo-hw --clobber-output
   ```
3. Same test file on native_sim.
   ```
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim -O /tmp/demo-ns --clobber-output
   ```

Say:
- The pytest file is the same in 2 and 3. So is `test_harness.c`, the `test_btn` backdoor.
- What differs is the test overlay. On the DK,
  `tests/.../boards/nrf5340dk_nrf5340_cpuapp.overlay` moves only `sw0` to `gpio_emul`
  and keeps the real LED. On native_sim, `app.overlay` moves both, since there is no LED.
- Board vs host is a command line argument, not a rewrite.
- Mention once: the repo ships agent skills, we look at them at 00:35.

Fallback: recorded video of steps 1 and 2. Step 3 is always live.

## 20:50 Why firmware resists testing (15, talk)

- Flash and printk loop. Cost of a late defect.
- Test pyramid, where it maps onto embedded and where it does not.
  `docs/concepts/testing-levels.md` has the levels, including emulated driver and on target.
- Legacy code with console-log-based testing. ESP-IDF's testing flow as a comparison.
- AI point: an assistant writes a hundred lines of test code in a second. Volume goes up,
  verification is the only thing that scales with it.
- Where the simulators sit: native_sim, QEMU, Renode, real board. One slide, no demo.

## 21:05 Host sim and ztest (30, ALL)

Goal: logic behind a seam, first ztest suite passing in seconds.

- Show: `apps/02-ztest/src/blink_logic.c` pulled out of `main.c`, and `tests/unit`.
  ```
  west twister -T apps/02-ztest/tests -p native_sim -O /tmp/tw02 --clobber-output
  ```
- Exercise **ex1**: `apps/02-ztest/exercises/ex1-write-a-test/src/main.c`. Write
  `test_five_presses_from_off` and delete `ztest_test_skip()`.
  ```
  west twister -T apps/02-ztest/exercises -p native_sim -O /tmp/ex1 --clobber-output
  ```
- Finish line: ✅ `app02.blink.ex1` passes, not skipped.
- Debrief: the press number goes in the assertion message so a failure says which press.
  The build only links `blink_logic.c`: no driver, no devicetree.
- Early finishers: `apps/02-ztest/EXERCISE.md` ★.

## 21:35 GPIO emulation (35, ALL)

Goal: drive an emulated button, catch the interrupt, assert the LED pin moved.

- `diff -r apps/02-ztest/src apps/03-emul-gpio/src` is empty. Say it.
- Walk `tests/emul/src/main.c`: `gpio_emul_input_set()`, `gpio_emul_output_get()`.
- Why `setup` and not `before`: `gpio_emul` fires callbacks from `pin_configure()`, so
  re-running init looks like a phantom press.
  ```
  west twister -T apps/03-emul-gpio/tests -p native_sim -O /tmp/tw03 --clobber-output
  ```
- Hands-on: `apps/03-emul-gpio/EXERCISE.md` ★ (change a press count or assert, watch
  it fail, put it back).
- Finish line: ✅ `app03.blink.emul` green.
- Debrief: real driver, real callback, real `blinky.c`. Only the pin is fake.

## 22:10 Test-only overlays (20, ALL)

Goal: a suite gets its own devicetree and reroutes an alias, app source untouched.

- Walk `apps/03-emul-gpio/tests/emul/app.overlay`: the emul controller, the two aliases,
  the comment about why both have to move (SIGSEGV otherwise).
- Exercise **ex2**: `apps/03-emul-gpio/exercises/ex2-broken-overlay`. One mistake in
  `app.overlay`.
  ```
  west twister -T apps/03-emul-gpio/exercises/ex2-broken-overlay -p native_sim -O /tmp/ex2 --clobber-output
  ```
  Error they will see: `__device_dts_ord_DT_N_ALIAS_sw0_P_gpios_IDX_0_PH_ORD undeclared`.
  The answer: `sw1` should be `sw0`.
- Finish line: ✅ `app03.blink.ex2` green.
- Debrief: read the macro name, it contains the alias. `build/zephyr/zephyr.dts` shows
  what the build actually got. Keep the time they took in mind for 00:35.

## 22:30 Break (30)

Help anyone still stuck on pre-work or ex1/ex2. Reset the board. Check the clock.

## 23:00 Twister (25, DEMO, they can follow)

Goal: one runner for every suite and every board.

- `testcase.yaml` side by side:
  - `apps/02-ztest/tests/unit`: `platform_allow`, tags
  - `apps/05-shell-pytest/tests/emul_button_toggle/testcase.yaml`: four platforms,
    `harness: pytest`, and the commented-out `harness: shell` with no Python
  - the `slow` marker in that suite's `pytest.ini`, selected with `--pytest-args="-m slow"`
- Filtering:
  ```
  west twister -T apps/ -p native_sim --test app05.shell.pytest -O /tmp/tws --clobber-output
  west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim --pytest-args="-m slow" -O /tmp/twslow --clobber-output
  west twister -T apps/ -p native_sim -t unit --exclude-tag exercise -O /tmp/twt --clobber-output
  ```
- Platform matrix, build only:
  ```
  west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim -p nrf5340dk/nrf5340/cpuapp --build-only -O /tmp/twm --clobber-output
  ```
- Artifacts: `twister-out/twister.json`, and per scenario `build.log`, `handler.log`,
  `twister_harness.log`. The same files CI uploads at 00:10.
- `--exclude-tag exercise` is why CI stays green with broken exercises in the tree.
- No coverage.

**Bonus, only if ahead:** Renode, app 12. Say it is experimental and can break. No
explanation beyond "same button test, emulated nRF52840 DK, Robot harness".
```
west twister -T apps/12-renode-twister -p nrf52840dk/nrf52840/renode --board-root $PWD/apps/12-renode-twister/boards -O /tmp/tw12 --clobber-output
```

## 23:25 Designing tests (10, talk)

- AAA with a blank line between: `apps/07-unit-conventions/tests/ztest/src/main.c`.
- Test size: unit, emulated driver, e2e. Pick the cheapest level that can see the bug.
- What to test first in inherited code: the logic that has already broken once, and
  whatever sits behind a devicetree alias.
- How much is enough: a green suite only says the things you tested still work.

## 23:35 pytest e2e (35, ALL)

Goal: drive a running device from outside over the shell.

1. (5, ALL) `04-pytest-basics`, plain pytest, no device. `2_fixture`: 3 functions,
   7 tests. `4_marking`: run it with and without `-m "not custom_slow"`.
   ```
   cd apps/04-pytest-basics/4_marking && pytest -v -m "not custom_slow"
   ```
2. (10, DEMO) `05`: `test_harness.c`, the `test_btn` backdoor compiled into the test
   image only, then `pytest/test_gpio_toggle.py` and the `slow` marker in `pytest.ini`.
   ```
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim -O /tmp/tw05 --clobber-output
   ```
   `CONFIG_SHELL_VT100_COLORS=n`, or the escape codes break `str.find()`. The `if` before
   `readlines_until`: on native_sim the LED line is already in `exec_command`'s output,
   on the DK deferred logging can print it after the prompt.
3. (15, ALL) Exercise **ex3**: `apps/05-shell-pytest/exercises/ex3-broken-e2e`. One bug
   in `src/main.cpp`.
   ```
   west twister -T apps/05-shell-pytest/exercises -p native_sim -O /tmp/ex3 --clobber-output
   ```
   Failure: 3 of 3 fail, each with `Did not find line "LED is now ON" within 2 seconds`.
   The answer: `led_state` starts `true` while the pin is configured inactive, so the
   first press prints OFF. Back to `false` (`solution/main.cpp`).
4. (5) Debrief. Finish line: ✅ suite green, Twister says `3 of 3 executed test cases passed`.
   - The assertion names the line it expected. `handler.log` shows what the device
     printed instead, `LED is now OFF`.
   - The comment above `led_state` gives a plausible reason for starting it lit. A
     review would pass it, the test does not.
   - The failing test's log is `twister_harness.log`, short summary at the end.

## 00:10 CI (20, ALL)

Goal: their own repo runs Twister on every push and uploads the reports.

- (3) Walk `.github/workflows/test-native-sim.yml`: container image, one twister line,
  `--exclude-tag exercise`, the upload step with `if: always()`.
- (2) They break a passing test in their own repo, commit, push. For example flip one
  expected string in `apps/02-ztest/tests/unit/src/main.c`.
- (5) While it runs (~4-5 min): `sanitizers.yml` in one minute (32-bit runtimes, the
  `.config` assertion). Then the self-hosted runner: what it is, that it runs inside the
  devcontainer, `docs/guides/self-hosted-runner.md`. Mine is registered, theirs is
  optional pre-work.
- (5) Red run. Open the log, read from the top. Download the `twister-reports` artifact.
- (5) Revert, push. They do not need to wait for green.
- Finish line: ✅ red run seen and artifact downloaded.
- Keep one red run URL of my own for the skills block.

## 00:30 On-target CI (5, DEMO, board)

- `test-hardware.yml`: manual, `[self-hosted, linux]`, `concurrency: hardware`.
- Build and flash `01-blinky`, then Twister `--device-testing` on 05 with the hardware
  map. This is what ran the opening demo.
- The hardware map is the only place the probe serial and port are written down.
- Trigger it live if the runner is up, otherwise the screenshots in `docs/imgs/`.

## 00:35 Agent skills (15, DEMO)

Goal: how the skills are written, so they can rewrite them. Not an unattended agent.

1. (5) **Scaffold**, `zephyr-ztest`. Ask for a ztest suite over `blink_logic` or the
   feeder. Compare to the hand-written `tests/unit`. It scopes first, then writes AAA.
2. (6) **Diagnose**, `zephyr-build-run`, on **ex2b**
   (`apps/03-emul-gpio/exercises/ex2b-missing-kconfig`). They have not seen this one.
   The error is `undefined reference to '__device_dts_ord_24'`, which is unreadable. The
   skill should name `CONFIG_GPIO` missing and point at `prj.conf`. Compare with how
   long ex2 took by hand.
3. (2) **Triage**, `twister-triage`, on the red CI artifact from 00:10. One line per
   failure.
4. (2) Open one `SKILL.md`: the description is what triggers it, the tables of real
   error messages do the work, it ends by handing the decision back.

- Everything the skills produce also exists hand-written in the repo.
- No API key or particular assistant needed to attend (PI FAQ 3).
- Fallback: recording of 1 and 2. Model output varies, rehearse on the exact bugs.

## 00:50 Legacy project (10, talk)

Adoption path, one step at a time, each one mergeable on its own:
1. Find where the code binds to the board: the devicetree aliases.
2. Add a test-only overlay that moves those aliases onto `gpio_emul` (or a bus emulator).
3. Add a `testcase.yaml` with `platform_allow: native_sim`, one e2e check over a shell
   backdoor that lives in the test folder.
4. Add the CI job. Now every push runs it.
5. Only then pull logic behind a seam and unit test it, where a bug has already bitten.

Footnote: `docs/concepts/what-you-cannot-test.md`, what emulation does not model.

Q&A. Go back to what they said they wanted at 20:15.

---

## Pre-work (attendees)

Sent with the repo link. The README "Preparation" section is the guide.

1. Docker, VS Code, Dev Containers extension.
2. **Use this template** into their own GitHub account. No forks.
3. Clone, reopen in container, wait for the SDK download.
4. Build and run `00-hello`.
5. `west twister -T apps/ -p native_sim --exclude-tag exercise` green.
6. Actions tab of their repo: Tests (native_sim) and Sanitizers green.
7. Optional: flash `01-blinky` to their own board.
8. Optional: register a self-hosted runner, `docs/guides/self-hosted-runner.md`.

## Prep (me)

**Before the pre-work goes out**
- [ ] Merge `feat/workshop-exercises` to main. The template copies main.
- [ ] Create a repo from the template with a fresh account or org, run the whole
      pre-work on it, time CI.
- [ ] Pre-work step 5 on a slow Windows laptop. Time it.
- [ ] Confirm both images still pull anonymously
      (`zephyr-devcontainer-ci`, `zephyr-devcontainer-devel`, tag `z4.4.0-sdk1.0.1`).

**The day before**
- [ ] Board demo end to end on the DK, including LED1 blinking during the pytest run.
- [ ] Record fallbacks: board demo, skills block 1 and 2.
- [ ] Self-hosted runner up inside the devcontainer, `test-hardware.yml` green once.
- [ ] Skills demo rehearsed on ex2b, twister-triage on a real red artifact.
- [ ] Reset my own checkout so ex1, ex2, ex2b, ex3 are broken again.
- [ ] Renode app 12 once, to know whether it is offered at all.

**30 min before**
- [ ] Container up, SDK volume present, one native_sim build warm.
- [ ] Board plugged in, camera framed on LED1 and button 1, serial port free.
- [ ] Tabs: repo, my template repo's Actions tab, this file, recordings.
- [ ] Terminal font size up, notifications off.
- [ ] Chat snippets ready to paste: every command in this file.

## Known errors

| Symptom | Fix |
|---|---|
| `OSError: [Errno 39] Directory not empty` | add `-O /tmp/<name> --clobber-output` |
| `west: command not found` | not inside the container, or SDK still downloading |
| three failures in a repo-wide run | missing `--exclude-tag exercise` |
| Actions tab empty on their repo | `git commit --allow-empty -m "start CI" && git push` |
| image pull hangs or is slow | corporate proxy or VPN. Watch Part 1, fix in the break. |

The rest: `docs/setup/first-run.md`, `docs/troubleshooting.md`.

---

## PI check

What the PI promises and where it stands in the repo. For me, before the day.

| PI promise | Status |
|---|---|
| Same test on board and native_sim, difference is the overlay | 05, with the nRF5340 DK test overlay. Only the DK has one. |
| Devicetree seam, emulation, test-only overlays | 03, ex2 |
| AAA, test sizing, what to test first | 07, `testing-levels.md`, talk |
| Twister: filtering, matrices, artifacts | 02, 05 testcase.yaml. No config matrix example. |
| Twister coverage | dropped |
| pytest e2e with fixtures and markers | 04, 05, ex3 |
| CI on push with artifacts uploaded | `test-native-sim.yml`, `twister-reports` on every run |
| CI "on your own fork" | template repo instead |
| Self-hosted runner running host sim | `build-check.yml` self-hosted job (manual, upstream hello_world). Shown, not run by attendees. |
| On-target CI | `test-hardware.yml` |
| Two agent skills | `zephyr-ztest`, `zephyr-build-run` |
| "A few smaller diagnostic" skills | `twister-triage` |
| Incremental adoption strategy | talk only, the checklist above. No page in `docs/`. |
| Dockerfile in the repo | not in this repo, the image comes from ghcr. Fine. |
| Bonus exercises: fakes, I2C emul, sensor emulators | 06, 08, `EXERCISE.md` in every app. 09 to 12 too. |
| Final files shared afterwards | the repo. `solution/` folders are already in it. |
