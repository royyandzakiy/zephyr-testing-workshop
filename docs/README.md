# docs

Everything in this repository that is not one of the examples: setup, commands,
concepts, and the failures you are likely to run into. Where a page could be aimed at
either a learner or a maintainer it is aimed at the learner, and the maintainer version
goes in [`notes/`](notes/).

The examples themselves are in [`../apps/`](../apps/). Each one has its own `README.md`
explaining what it is and how to run it, and an `EXERCISE.md` with more to try. These
docs are what sits around them.

## Get it running

1. [`setup/`](setup/) for the environment. The devcontainer is the supported path.
2. `apps/00-hello`, which checks the toolchain.
3. [`setup/first-run.md`](setup/first-run.md) if that did not work.

## Learn one technique

Each of these is one app you can read and run on its own.

| You want | Go to |
|---|---|
| tests that run on the device, in C | [`apps/02-ztest`](../apps/02-ztest), then [`apps/07-unit-conventions`](../apps/07-unit-conventions) for how to write them well |
| faking a pin or a chip | [`apps/03-emul-gpio`](../apps/03-emul-gpio), then [`apps/06-sensor`](../apps/06-sensor) for a real driver over a fake bus |
| faking the functions your code calls | [`apps/08-fff-mocks`](../apps/08-fff-mocks), and [`apps/09-gtest-gmock`](../apps/09-gtest-gmock) for the C++ version |
| driving the device from outside | [`apps/04-shell-pytest`](../apps/04-shell-pytest), then [`apps/05-pytest-advanced`](../apps/05-pytest-advanced) |
| running any of it in CI | [`guides/ci.md`](guides/ci.md) for what each workflow does, [`guides/self-hosted-runner.md`](guides/self-hosted-runner.md) to register a runner |

If you are not sure which level a test of yours belongs at, read
[`concepts/testing-levels.md`](concepts/testing-levels.md) first.

## Look something up

| | |
|---|---|
| [`reference/boards.md`](reference/boards.md) | build, flash and Twister commands, per board, and what each flag does |
| [`reference/native-sim.md`](reference/native-sim.md) | the two board targets, where the console output goes, and the binary's command line options |
| [`reference/pytest-harness.md`](reference/pytest-harness.md) | `twister_harness`, the `dut` and `shell` fixtures, `harness_config` |
| [`troubleshooting.md`](troubleshooting.md) | failures keyed by what you are looking at |
| [`glossary.md`](glossary.md) | the vocabulary these docs assume |

## Understand why

| | |
|---|---|
| [`concepts/testing-levels.md`](concepts/testing-levels.md) | what each kind of test answers, and what it is blind to |
| [`concepts/what-you-cannot-test.md`](concepts/what-you-cannot-test.md) | the limits, and what a green suite does not mean |

The app READMEs carry their own version of these ideas in their Trivia sections, told
through the app in front of you. The pages here are the standalone version.

## Working material

[`notes/`](notes/) holds planning, a task list and an outline. It is kept because it is
useful to whoever maintains this repository. Nothing in it is finished and some of it
is out of date, which its own `README.md` and each file say at the top.
