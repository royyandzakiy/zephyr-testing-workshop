---
name: twister-triage
description: Read a finished twister run and name the cause of each failure in one line, without rebuilding anything. Works on a local twister output directory or on the twister-reports artifact downloaded from a GitHub Actions run. Use this when a twister run or a CI job has already failed and the user asks what broke, pastes a twister.json, points at twister-out/ or an artifact zip, or wants a failed CI run summarised. For running builds, flashing, or anything that needs a new run, use zephyr-build-run instead.
---

# Twister triage

A small, read-only skill. It takes the output of a run that already happened and turns
it into one line per failed scenario: what failed, the cause, and where to look. It does
not build, run or edit anything. When the answer needs a rebuild, it says so and hands
over to `zephyr-build-run`.

## Where the input comes from

| Source | What is in it |
|---|---|
| `-O /tmp/<name>` from a local run | everything, including build directories |
| `twister-out/` in the repo | same, but see the bind mount note in `CLAUDE.md` |
| `twister-reports` artifact from `test-native-sim.yml` | `twister.json`, `twister_report.xml`, and every `build.log`, `handler.log`, `twister_harness.log`. **No build directories**, so no `.config` and no `zephyr.dts`. |

To fetch the artifact of the latest failed run, when `gh` is available:

```bash
gh run download --name twister-reports --dir /tmp/tr
```

## Step 1: read twister.json, nothing else yet

`twister.json` has one entry per scenario under `testsuites`. The fields that matter are
`name`, `platform`, `status`, `reason`, and `log` (the tail of the build or run log,
already inlined). Each entry also has `testcases`, each with its own `status` and
`reason`.

```bash
python3 - /tmp/tr/twister.json <<'EOF'
import json, sys
d = json.load(open(sys.argv[1]))
for s in d["testsuites"]:
    if s["status"] in ("passed", "skipped", "filtered", "not run"):
        continue
    print(f'{s["name"]}  [{s["platform"]}]  {s["status"]}: {s.get("reason")}')
    for t in s.get("testcases", []):
        if t["status"] in ("failed", "error"):
            print(f'    {t["identifier"]}: {t.get("reason")}')
EOF
```

`status` tells you which phase failed, which decides where to look next:

| `status` | Phase | Look in |
|---|---|---|
| `error` with reason `Build failure - ...` | cmake, devicetree, Kconfig, compile or link | the suite's `build.log`, or the `log` field |
| `failed` on a ztest scenario | the test ran and an assertion fired | `handler.log`, search for `FAIL` and `Assertion failed` |
| `failed` on `harness: pytest` | pytest ran and a Python assertion fired | `twister_harness.log`, the `short test summary info` block at the end |
| `failed` with reason `Timeout` | the image never printed the end of suite marker | `handler.log`, last lines. Usually a crash, a hang, or a console on the wrong UART. |
| `blocked` on a test case | its suite did not build | ignore it, triage the suite |

## Step 2: match the reason to a cause

These are the messages this repo produces in practice. The first two are what
`apps/03-emul-gpio/exercises/ex2-broken-overlay` and `ex2b-missing-kconfig` fail with.

| Text in `reason` or the log | Cause | Confirm with |
|---|---|---|
| `__device_dts_ord_DT_N_ALIAS_<name>_P_...` undeclared | the alias `<name>` does not exist in this build's devicetree. A typo in an overlay, or an overlay that did not get picked. | `grep -A20 'aliases {' <build>/zephyr/zephyr.dts` |
| ``undefined reference to `__device_dts_ord_<N>'`` | the node exists but no driver for it was compiled. A missing `CONFIG_` symbol, usually `CONFIG_GPIO`, `CONFIG_I2C` or `CONFIG_SENSOR`. | `grep -E '^ \*\s+<N>\s' <build>/zephyr/include/generated/zephyr/devicetree_generated.h` names the node, then `grep <SYMBOL> <build>/zephyr/.config` |
| `XPASS(strict)` | a test marked `xfail(strict=True)` passed. The code changed a behaviour the suite records as known. | the `reason` string on the marker, and the comment it points to |
| `SIGSEGV` in `handler.log` on native_sim | a driver call on a device that was never initialised. In this repo, almost always one alias rerouted to `gpio_emul` and its partner left on native_sim's own `&gpio0`. | the comment in `apps/03-emul-gpio/tests/emul/app.overlay` |
| `OSError: [Errno 39] Directory not empty` | twister rotating its output on the Windows bind mount, not a test failure | rerun with `-O /tmp/<name> --clobber-output` |

The confirm column needs a build directory. The CI artifact has none, so for a CI
failure either reason from the log alone or reproduce the one scenario locally:

```bash
west twister -T apps/ -p native_sim -s <scenario> -O /tmp/tr-local --clobber-output
```

## Step 3: report

One block per failed scenario, in this shape, and nothing longer:

```
app03.blink.ex2b  [native_sim/native]  build failure (link)
  cause:  CONFIG_GPIO is not set, so the gpio_emul node (ordinal 24) has no driver
  where:  apps/03-emul-gpio/exercises/ex2b-missing-kconfig/prj.conf
  fix:    add CONFIG_GPIO=y
```

Say "cause not certain" when it is not, and name the file you would read next. Do not
paste the log back. The user has the log already, and the point of this skill is to
save them from reading it.

Propose the fix and stop. The user decides whether to apply it.
