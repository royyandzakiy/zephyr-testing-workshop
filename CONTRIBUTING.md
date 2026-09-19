# Contributing

For anyone proposing a change to this repository: a fix to an app, a new test, a
correction to the docs, or a new app folder.

Most of what arrives here will be a correction. The apps are teaching material, so a
README that describes output the app no longer prints is as much of a bug as a test
that fails.

## Getting an environment

Everything builds inside the devcontainer, so Docker is the only prerequisite. Open the
repo in VS Code and accept *"Reopen in Container"*. The first start downloads Zephyr and
the SDK into a shared Docker volume and takes a while.
[`docs/setup/devcontainer.md`](docs/setup/devcontainer.md) covers what the container
does and what to do when it does not come up.

## Running the tests

One command runs every suite in the repo on `native_sim`:

```bash
west twister -T apps/ -p native_sim -O /tmp/tw --clobber-output
```

The `-O /tmp/tw --clobber-output` part matters on Windows. Twister rotates
`twister-out/` into `twister-out.1` on each run, and that rotation fails on a Windows
bind mount with `OSError: [Errno 39] Directory not empty`. Writing the output inside the
container avoids it.

One app at a time:

```bash
west twister -T apps/07-unit-conventions -p native_sim -O /tmp/tw --clobber-output -v
```

[`docs/reference/boards.md`](docs/reference/boards.md) has the build, flash and Twister
commands for each board in the tree.

## Before you open a pull request

1. **Build it.** Host `gcc` will verify pure C logic, and that is worth doing first, but
   it says nothing about Kconfig, devicetree, ztest ordering or the Twister harness.
   Those only appear in a real build.
2. **Run the full suite**, not only the app you touched. Suites share overlays and
   Kconfig fragments.
3. **Show the test failing.** If you are adding a test, break the thing it is meant to
   catch and confirm which assertion fires.
4. **Report the numbers from the run**, not an impression of it. Paste the Twister
   summary line.

If a change cannot pass on `native_sim` because it needs a board, say which board and
what you saw on it.

## Writing docs

Every app carries a `README.md` and an `EXERCISE.md`, and `docs/` carries everything
that is not about one app. [`docs/README.md`](docs/README.md) is the index.

The rules, in short:

- **No em dashes**, anywhere. Not the unicode character and not a doubled hyphen. Use a
  comma, a spaced hyphen, parentheses, or two sentences.
- **No workshop or business context under `apps/`.** No "session 2", no "before the
  break", no "the room". The apps have to make sense to someone alone with the
  repository. Cross-references between apps are fine and wanted.
- **One owner per fact.** An app README owns how to run that app. `docs/reference/` owns
  how the tool works in general. Where they would overlap, the README links.
- **Every `*Check:*` line in an `EXERCISE.md` has to be something the reader can
  literally do**, with the command spelled out and some idea of what the answer looks
  like.

`apps/00-hello/README.md` and `apps/00-hello/EXERCISE.md` are the reference pair. Read
them before writing a new one. The full rules live in the `repo-docs` skill under
[`.claude/skills/`](.claude/skills/).

## Code style

4 spaces, no tabs, 100 columns. [`.editorconfig`](.editorconfig) and
[`.clang-format`](.clang-format) record it.

This differs from Zephyr upstream, which uses tabs. It is what most of the C in `apps/`
already does, and it is deliberate.

The repo is not yet consistent with it. Apps 07 and 08 are tab-indented, and two apps
are internally split. No reformatting pass has been run, so do not run one as part of an
unrelated change. Match the file you are editing. When the formatting pass does happen,
it is this:

```bash
git ls-files '*.c' '*.h' '*.cpp' '*.hpp' | xargs clang-format -i
```

Comments explain **why**, particularly where a line exists to avoid a specific bug.
[`apps/07-unit-conventions/src/feeder.c`](apps/07-unit-conventions/src/feeder.c) is the
model: the comment names the tempting wrong version and says what it would cost.

## Adding an app

Each folder under `apps/` is a complete, standalone Zephyr application with its own
tests. Twister finds suites by recursing for `testcase.yaml`, so there is no registry to
update. Scenario names follow `appNN.<domain>.<kind>`, for example `app07.feeder.ztest`.

The layout is in [`CLAUDE.md`](CLAUDE.md), along with the conventions this repo expects
of anything generated with Claude Code.

## Reporting a problem

If the container will not come up or an app will not build,
[open a setup issue](https://github.com/royyandzakiy/zephyr-testing-workshop/issues/new?template=setup-problem.yml). The form asks for the
things needed to reproduce it. For anything else, a plain issue with the command you ran
and its full output is enough.
