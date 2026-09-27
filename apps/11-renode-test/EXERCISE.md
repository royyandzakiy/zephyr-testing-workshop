# 11-renode-test - exercises

## ★ warm-up

1. **Open the report.** After a run, `build/log.html` has every keyword of every case,
   with its arguments and how long it took. List what the run wrote:

   ```bash
   ls build/log.html build/report.html build/robot_output.xml
   ```

   *Check:* open `build/log.html`, expand `Should Turn The LED On When sw0 Is Pressed`,
   and you can name the keyword that took the longest.

2. **Make a case fail on purpose.** In `button_led.robot`, change the first
   `Assert LED State  false` to `true`, then run the suite again:

   ```bash
   cd build && renode-test ../button_led.robot
   ```

   *Check:* you can say what Robot reported for that case, and that the other case
   still passed. Put the line back when you are done.

3. **Take out the first press and release.** In `Boot The DK`, delete the two
   `sysbus.gpio0.sw0` lines at the end, and run the suite again.

   *Check:* both cases fail, and you can find the `Wait For Line On Uart` line each one
   stopped at in the report `renode-test` prints. Put the lines back when you are done.

## ★★ go deeper

1. **Press the button by hand.** Start the machine with the command from the README,
   then at the `(nrf52840)` prompt:

   ```
   sysbus.gpio0.sw0 Press
   ```

   ```
   sysbus.gpio0.sw0 Release
   ```

   Do it three times.

   *Check:* the first press prints nothing, the next two print `LED is now ON` and
   `LED is now OFF`, and you can say why the first one is lost.

2. **Hold the button down.** Add a third case that presses `sw0`, waits for
   `LED is now ON`, and then checks that nothing else is printed while it stays
   pressed:

   ```
   Should Not Be On Uart     Button pressed!    timeout=1
   ```

   *Check:* the case passes, and you can name the line in `src/main.c` that makes a
   held button count as one press.

3. **Use Renode's own DK description.** In `run_nrf52.resc`, replace the `set platform`
   block and the `LoadPlatformDescriptionFromString` line with:

   ```
   machine LoadPlatformDescription @platforms/boards/nrf52840dk_nrf52840.repl
   ```

   Run the suite again.

   *Check:* you can say what the firmware sees on pin 11 at rest now, and why a
   `Press` in Renode no longer means a press on the DK. Put the lines back when you are
   done.

## ★★★ off the map

1. **Wire up `sw1` and `led1`.** Add them to `run_nrf52.resc` on the DK's pins, give the
   app a second button that drives the second LED, and test both.

   *Why it is interesting:* the `.resc` is the only description of the board Renode
   has, so every pin the test touches has to be in it, with the right polarity.

2. **Fix the GPIOTE model in Renode.** The model is `NRF52840_GPIOTasksEvents.cs` in the
   `renode-infrastructure` repository. Find where it records the pin level when a
   channel is set up, and what overwrites it straight after.

   *Why it is interesting:* the first press is lost because of the order two parts of
   one register write are handled in, and a fix upstream would make the press and
   release in `Boot The DK` unnecessary.

## If you want to go further

- [Renode testing with Robot](https://renode.readthedocs.io/en/latest/introduction/testing.html) - the terminal tester, the LED tester and the other keywords Renode adds.
- [Robot Framework User Guide](https://robotframework.org/robotframework/latest/RobotFrameworkUserGuide.html) - keywords with arguments, and test setup and teardown.
- [`apps/12-renode-twister`](../12-renode-twister) - the same test, run by `west twister`, and what Twister needs from a board before it will run it.
