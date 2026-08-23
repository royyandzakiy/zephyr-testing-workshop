## Communicating with Native Sim

- By default native sim will create a pseudo tty (PTY) that you can find with `ls /dev/tty*`
- it allocates said PYT once you run the zephyr.exe

```bash
west build -p always -b native_sim/native -s <app> -d build
./build/zephyr/zephyr.exe
python3 -m serial.tools.miniterm --raw /dev/pts/3 115200

#or, in one line
./build/zephyr/zephyr.exe --attach_uart_cmd="python3 -m serial.tools.miniterm --raw %s 115200"
```

```bash
uart connected to pseudotty: /dev/pts/3
*** Booting Zephyr OS build v4.2.2 ***
...
```

- Use this to remove the PTY, and instead run the `zephyr.exe` with uart just like a console app with the console ready
- by default, this is set to `n`, so no need to be explicit

```bash
CONFIG_UART_NATIVE_PTY_0_ON_STDINOUT=y
```