# native_sim

`native_sim` compiles Zephyr as an ordinary program for the machine you are sitting on.
The build produces `build/zephyr/zephyr.exe`, and you run that like any other
executable. Verified against Zephyr v4.4.2.

## The two board targets

| Board target | Is |
|---|---|
| `native_sim/native` | 32-bit. This is what `native_sim` on its own resolves to. |
| `native_sim/native/64` | 64-bit, same board with the `64` variant |

The 32-bit build links against 32-bit host libraries. When those are missing the link
fails on a multilib error, and `native_sim/native/64` will build in the same place.
The sanitizer runtimes follow the same split: `lib32asan` and `lib32ubsan` for
`native_sim/native`, the amd64 packages for the 64-bit variant.

## Where the console output goes

There are two places `printk` output can end up, and which one you get depends on
whether the image has a UART at all.

**Without `CONFIG_SERIAL`**, output goes to the stdout of the shell that launched
`zephyr.exe`. Nothing is configurable and nothing has to be attached. This covers
most of the apps in this repo, since a ztest suite needs no UART.

**With `CONFIG_SERIAL=y`**, the `zephyr,native-pty-uart` node in the devicetree brings
in `CONFIG_UART_NATIVE_PTY`, and a choice appears for where the first UART is
connected:

```bash
# zephyr.exe behaves like a console application, reading and writing your terminal
CONFIG_UART_NATIVE_PTY_0_ON_STDINOUT=y
```

```bash
# zephyr.exe allocates a pseudo terminal and the UART lives on that instead
CONFIG_UART_NATIVE_PTY_0_ON_OWN_PTY=y
```

`ON_OWN_PTY` is the Kconfig default (`drivers/serial/Kconfig.native_pty`), so
`ON_STDINOUT` has to be set explicitly. Every app in this repo sets it, in
`boards/native_sim_native.conf`.

### Setting them on an image with no UART does nothing

`Kconfig.native_pty` is sourced inside `if SERIAL` in `drivers/serial/Kconfig`. With
`CONFIG_SERIAL` off, neither symbol exists, so it will not appear in
`build/zephyr/.config` and the line in your `.conf` file is inert. It is not an error
and nothing warns about it.

To find out which one a build actually got:

```bash
grep UART_NATIVE_PTY build/zephyr/.config
```

No output means the image has no UART and the console is on stdout already. In this
repo the only builds where the setting does anything are the shell suites,
`apps/04-shell-pytest/tests/drivers/gpio_button_toggle` and
`apps/05-pytest-advanced/tests/shell_pytest`.

## Running with its own PTY

The pseudo terminal is allocated when the process starts, and the path is printed on
the first line:

```bash
./build/zephyr/zephyr.exe
```

```
uart connected to pseudotty: /dev/pts/3
```

The process keeps running with nothing on the other end until you attach to that path
from a second terminal:

```bash
python3 -m serial.tools.miniterm --raw /dev/pts/3 115200
```

`--raw` stops control characters being mangled. Without it the shell prompt and any
ANSI escapes render wrong.

The path changes every run, so the binary can open the terminal for you instead:

```bash
./build/zephyr/zephyr.exe --attach_uart_cmd="python3 -m serial.tools.miniterm --raw %s 115200"
```

`%s` is where the pty path is substituted, and there has to be exactly one. Passing
`--attach_uart` on its own uses the command in
`CONFIG_UART_NATIVE_PTY_AUTOATTACH_DEFAULT_CMD`, which defaults to
`xterm -e screen %s &`.

## Command line options

`zephyr.exe --help` lists them. The set depends on what the image was built with, so
two builds of the same app can print different lists.

| Option | Does |
|---|---|
| `--stop_at=<seconds>` | exit after this much simulated time |
| `--rt` / `--no-rt` | run at host real time, or as fast as the simulation allows |
| `--rt-ratio=<ratio>` | run simulated time at a multiple of real time |
| `--rtc-offset=<seconds>`, `--rtc-reset` | where the simulated RTC starts |
| `--testargs <arg>...` | everything after this is left for the application to read |
| `--no-color` | no colour escapes in traces |

These three are added by the PTY UART driver, and only when
`CONFIG_UART_NATIVE_PTY_0_ON_OWN_PTY` is set. On an `ON_STDINOUT` build they are
absent from `--help` and rejected if you pass them:

| Option | Does |
|---|---|
| `--attach_uart` | run the default attach command against the new pty |
| `--attach_uart_cmd="<cmd>"` | run this command instead, with `%s` as the pty path |
| `--wait_uart` | hold UART writes until something attaches to the pty |

Simulated time is not host time. With `--no-rt`, which is the default, a
`k_sleep(K_SECONDS(10))` returns as fast as the host can get there.

## References

| | |
|---|---|
| [native_sim board docs](https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html) | the board itself, its limitations, and the full command line option list |
| [PTY UART](https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html#pty-uart) | the section on the two connection modes |
| `$ZEPHYR_BASE/drivers/serial/Kconfig.native_pty` | the four symbols and their help text |
| [`boards.md`](boards.md) | the build and run commands for native_sim alongside the other boards |
