# When the first build fails

The checks below are ordered by how often they turn out to be the cause, so work down
the list. They all assume the Preparation steps in the root
[`README.md`](../../README.md) have been done.

The command that should work is:

```bash
cd apps/00-hello && west build -b native_sim/native -p && ./build/zephyr/zephyr.exe
```

and it should print:

```
*** Booting Zephyr OS build v4.4.2 ***
Hello from the Zephyr testing workshop!
Board: native_sim/native
```

## 1. Are you inside the container?

Everything here builds in the devcontainer. `west` does not exist on the host.

```bash
echo $ZEPHYR_BASE
```

Nothing printed means you are on the host. In VS Code, reopen with
*Dev Containers: Reopen in Container* from the command palette.

## 2. Did the SDK finish downloading?

The first container start pulls Zephyr and the Zephyr SDK into a shared Docker volume
and prints a `FIRST RUN ON THIS MACHINE` banner. If you opened a terminal while that
was still running, `west` will be there but the toolchain will not.

```bash
ls $ZEPHYR_BASE/VERSION && west sdk list
```

If either fails, wait for the banner to finish and open a new terminal.

## 3. `west: command not found`

The container started but the postStart script has not run. Check it:

```bash
ls /opt/devcontainer/setup-sdks.sh
```

If it is there, run it by hand and watch for errors. If it is not, the wrong image is
running. See [`devcontainer.md`](devcontainer.md).

## 4. `board not found` or a devicetree error

Check the board name is spelled with its qualifiers. `native_sim` alone is a board;
`native_sim/native` names the SoC as well, and that is what this repo uses.

```bash
west boards | grep native_sim
```

A devicetree error naming `DT_ALIAS` or a node label usually means an overlay did not
get picked up. Overlay discovery is relative to `-s`, so building the wrong directory
is the usual cause. See
[`../reference/boards.md`](../reference/boards.md#overlay-auto-discovery).

## 5. It builds but prints nothing

`native_sim` can put its console on your terminal or on a pseudo-terminal. Every app
here is configured for the first. A binary that prints
`uart connected to pseudotty: /dev/pts/3` and then nothing else got the second, and is
waiting for you to attach a terminal to that path.
[`../reference/native-sim.md`](../reference/native-sim.md) covers both modes and how to
check which one a build got.

## 6. Stale build directory

Symptoms are confusing and rarely mention the build directory. If you have built this
app for another board in the same directory:

```bash
west build -b native_sim/native -p always
```

`-p` wipes it. Leaving `-p` off after changing board is a common cause of errors that
look like something else.

## 7. The test suite fails but the app runs

```bash
west twister -T apps/ -p native_sim
```

On Windows this can fail before anything builds, with
`OSError: [Errno 39] Directory not empty`. That is Twister rotating `twister-out/` on
the bind mount. Give it another output directory:

```bash
west twister -T apps/ -p native_sim -O /tmp/tw --clobber-output
```

For failures after the build, see [`../troubleshooting.md`](../troubleshooting.md).

## Still stuck

Open an issue on the repository with the full output of the failing command and the
output of:

```bash
west --version && echo $ZEPHYR_BASE && west sdk list
```
