# 06-sensor - exercises

## ★ warm-up

1. **Trip the alarm.** Change `BME280_EMUL_ADC_TEMP_DEFAULT` in
   `src/sensors/bme280_emul.h` so the app reports above 30 degC. Write down the
   temperature you expect first, then run it:

   ```bash
   west build -b native_sim/native -p && ./build/zephyr/zephyr.exe
   ```

   *Check:* `ALARM` flips, and your prediction was within a degree. Put the value back
   when you are done.

2. **Find the hysteresis band.** Humidity sits at 65.104 %RH, which is 0.1 above the
   release point. Nudge `BME280_EMUL_ADC_HUM_DEFAULT` up until the alarm latches, then
   bring it back down. The two points that decide it are here:

   ```bash
   grep CLIMATE_ src/sensors/climate_logic.h
   ```

   *Check:* the alarm does not clear at the same value it tripped at, and you can name
   which of those four constants each edge used. Put the value back when you are done.

3. **Corrupt the chip ID.** Change `CHIP_ID` in `src/sensors/bme280_emul.c` from
   `0x60` to `0x58`, then run the emulated suite:

   ```bash
   west twister -T apps/ -p native_sim --test app06.climate.emul
   ```

   The driver rejects it. Find where:

   ```bash
   grep -n "BME280_CHIP_ID\|ENOTSUP" $ZEPHYR_BASE/drivers/sensor/bosch/bme280/bme280.c
   ```

   *Check:* you can name the error code and the line that returns it. Put the ID back
   when you are done.

4. **Break the humidity nibble packing.** In `load_calibration()`, swap the two halves
   of the `h[4]` encoding and run the suite again.

   *Check:* you can name which assertion caught it, or say that none of them did. Both
   answers are worth having, and the second one tells you the suite has a hole. Put it
   back when you are done.

## ★★ go deeper

1. **Add a channel.** The emulator serves an 8-byte burst. Make `bme280_read_once()`
   also report something derived, dew point for instance, and test it in `tests/unit`
   where there is no bus involved.

   *Check:* the new test needed no changes at all in `tests/emul`.

2. **Test the error paths.** Make the emulator return `-EIO` on the status register
   read. Assert that `bme280_read_once()` propagates it. Then look at how the next app
   does the same thing:

   ```bash
   grep -n "read_fails_eio" -A 6 ../08-fff-mocks/tests/fff/src/main.c
   ```

   *Check:* you can say how many lines each version took, and which you would rather
   write again.

3. **Run the sanitizers.**

   ```bash
   west twister -T apps/06-sensor -p native_sim --enable-asan --enable-ubsan
   ```

   *Check:* you get the left-shift-of-negative report at `bme280.c:103`, you have read
   what [`NOTES.md`](NOTES.md) says about it, and you have an opinion about what to do
   with a finding in a vendor driver you do not own.

4. **Compare what the two suites cost.** Build each one and look at the size report:

   ```bash
   west build -b native_sim/native -p -s apps/06-sensor/tests/unit -d build_u -t rom_report
   ```

   ```bash
   west build -b native_sim/native -p -s apps/06-sensor/tests/emul -d build_e -t rom_report
   ```

   *Check:* you can say roughly what the driver, the I2C stack and the emulation
   framework each cost, and why that matters for how often you run each suite.

## ★★★ off the map

1. **Implement the `emul_sensor_driver_api` backend.** `bme280_emul.h` explains why
   this repo passes `NULL` for it. Implement it, and make a test ask for "25.5 degC"
   instead of a raw ADC code.

   *Why it is interesting:* it is nicer to use, and it requires inverting Bosch's
   compensation, at which point you have to decide whether the test is still testing
   the driver or is now testing your inverse.

2. **Emulate a chip that is flaky rather than broken.** Make reads succeed 90% of the
   time. Decide what the application should do, then make it do that.

   *Why it is interesting:* "retry" is the obvious answer and it is wrong often enough
   to be worth arguing about. The retry policy is also the part you will most want to
   test, and it lives in the application rather than in the driver.

3. **Write an emulator for a part you actually use.** Pick one from your own work, find
   its register map, and get as far as a chip ID read.

   *Why it is interesting:* the second emulator you write takes an afternoon rather
   than a week.

4. **Decide whether the UBSAN finding is a bug.** Read the compensation code and the
   datasheet section it was copied from:

   ```bash
   sed -n '85,115p' $ZEPHYR_BASE/drivers/sensor/bosch/bme280/bme280.c
   ```

   Then write the three sentences you would put in an upstream issue, or the three
   sentences explaining why you would not open one.

   *Why it is interesting:* there is no right answer, and having to write it down is
   the exercise.

## If you want to go further

- [Emulator API](https://docs.zephyrproject.org/latest/hardware/emulator/index.html) - `EMUL_DT_INST_DEFINE`, backend APIs, and the emulated bus controllers.
- [Zephyr's sensor emulator tests](https://github.com/zephyrproject-rtos/zephyr/tree/main/tests/drivers/sensor) - a large collection of worked examples, and the place to take conventions from.
- [Sensor API](https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html) - the `sensor_value` convention, and which `SENSOR_CHAN_*` are defined.
- [`docs/concepts/testing-levels.md`](../../docs/concepts/testing-levels.md) - what an emulated driver test answers, and what it is blind to.
