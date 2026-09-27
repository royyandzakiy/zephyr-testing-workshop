# 11-renode-twister - exercises

## ★ warm-up

1. **Find the result of each Robot case.** After a run, the per-case lines are in
   `handler.log`:

   ```bash
   grep "Finished test" $(find /tmp/tw11 -name handler.log)
   ```

   *Check:* you can name the three cases, and say which one took the longest and why it
   needs more emulated time than the other two.

2. **Make a case fail on purpose.** In `tests/robot/button_toggle.robot`, change
   `Assert LED State  false` under `Should Boot With The LED Off` to `true`, then run
   the suite again with the command from the README. Find the failure log:

   ```bash
   find /tmp/tw11 -name "*.fail0.log"
   ```

   *Check:* you can say what Robot reported for that case, and that the other two
   cases still passed. Put the line back when you are done.

3. **Take the workaround out and watch the first press go missing.** In `Boot The DK`,
   put a `#` in front of `Resample The Button Pin`, and run the suite again.

   *Check:* you can name the cases that fail, and the `Wait For Line On Uart` line each
   one stopped at. Put the line back when you are done.

## ★★ go deeper

1. **Compare the variant with the stock board.** From `apps/11-renode-twister`, build
   both:

   ```bash
   west build -b nrf52840dk/nrf52840 -p -d build_stock
   ```

   ```bash
   west build -b nrf52840dk/nrf52840/renode -p -d build_renode
   ```

   ```bash
   diff build_stock/zephyr/.config build_renode/zephyr/.config
   ```

   *Check:* every line in the diff is a `CONFIG_BOARD` symbol, and you can say which
   file in `boards/nordic/nrf52840dk/` is the reason Twister treats the two differently.

2. **Hold the button down.** Add a fourth case that presses `sw0`, waits for
   `LED is now ON`, and then checks that nothing else is printed while the button stays
   pressed:

   ```
   Should Not Be On Uart     Button pressed!    timeout=1
   ```

   *Check:* the case passes, and you can say which line of `src/main.c` makes a held
   button count as one press.

3. **Give `--board-root` as a relative path.** Run the suite with
   `--board-root apps/11-renode-twister/boards` in place of the `$PWD` version, then
   read the log:

   ```bash
   grep "File does not exist" $(find /tmp/tw11 -name handler.log)
   ```

   *Check:* you can say which process looked for the `.resc`, which directory it was
   running in, and why Twister itself found the file.

## ★★★ off the map

1. **Wire up `sw1` and `led1`.** Add them to `boards/nrf52840dk.resc` on the DK's pins,
   give the app a second button that drives the second LED, and test both from Robot.

   *Why it is interesting:* the `.resc` is the only description of the board Renode
   has, so every pin the test touches has to be in it, with the right polarity.

2. **Fix the GPIOTE model in Renode.** The model is `NRF52840_GPIOTasksEvents.cs` in
   the `renode-infrastructure` repository. Find where the `OUTINIT` field and the
   register's write callback both set the channel's stored level, and decide which one
   should win in event mode.

   *Why it is interesting:* the workaround in `button_toggle.robot` exists because of
   the order two callbacks run in, and a fix upstream would make the keyword
   unnecessary for everyone.

3. **Run the suite in CI.** Add a job to `.github/workflows/` that runs the Twister
   command from the README in the `ci` image.

   *Why it is interesting:* the job needs no board and no self-hosted runner, and it
   still runs the image you would flash.

## If you want to go further

- [Renode testing with Robot](https://renode.readthedocs.io/en/latest/introduction/testing.html) - the terminal tester, the LED tester and the other keywords Renode adds.
- [Robot Framework User Guide](https://robotframework.org/robotframework/latest/RobotFrameworkUserGuide.html) - keywords with arguments, `FOR` and `IF`, and test setup and teardown.
- [Board extensions](https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html) - adding a variant to a board you do not own, which is what `boards/nordic/nrf52840dk/` does.
