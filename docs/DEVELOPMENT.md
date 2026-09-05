## To Do

- test all current apps/ working fine
    - 06: assemble board + bme280, ensure works fine. currently fails to compile
- add new apps
    - 05: advanced and variative pytest usage
    - 06* refine: will be about board bringup (bme280) to native sim
    - XX about finding seams in legacy projects
    - XX about unit test conventions
    - XX* using fff to mock functions
    - XX* using gtest gmock with a cpp project
    - XX* custom sensor api + native sim fake equivalent
    - XX create large project, portray good unit test coverage (will be called in ci with lcov and coverage report)
    - XX create simple project that will fail in sanitizer
    - XX about fuzz testing in ci
    - XX create bluetooth/wifi/usb cdc app, create tests for these hard to test things
- create claude md, claude skills:
    - build flash run* (incl run tests and summarize results or errors)
    - board bring up dts (incl board migration)
    - kconfig helper (debug kconfig issues accurately)
    - memory analyzer (binary size, largest symbol)
    - (ensure all use zephyr mcp where possible)
    - create n testing unit test*
    - create n testing pytest*
    - create ci: build, sanitize, run on ci, run self hosted
- ci
    - add ci lcov
    - fix sanitizer ci
    - add fuzzer ci

## Done

- apps/06 sensor spin: climate_logic seam + ztest, app-local BME280 i2c emulator + ztest
- cleaned up project NOTES

## Known Issue

- consider using clean image to reduce container size
- UBSAN trips inside Zephyr's own BME280 driver (bme280.c:103, left shift of a  negative value in the Bosch compensation formula). Upstream, not ours; see apps/06-sensor/NOTES.md.

## Docs

- update all NOTES
    - NOTES devcontainer
    - NOTES setup
    - NOTES zephyr vanilla ncs
    - NOTES pre build flash
    - NOTES pre general guide
    - WORKSHOP: refine with using ai
    - NOTES maintaining sdk toolchains + vscode ext
    - NOTES Notes testing theory, conventions and best practices