## To Do

- update all NOTES
    - NOTES devcontainer
    - NOTES setup
    - NOTES zephyr vanilla ncs
    - NOTES pre build flash
    - NOTES pre general guide
    - WORKSHOP
    - add: maintaining sdk toolchains + vscode ext
- test all apps/ working fine
- create claude md, claude skills:
    - testing unit test
    - testing pytest
    - ci

- apps/05 about pytest, a bit more advanced
- apps/07 do fault injection

## Done

- apps/06 sensor spin: climate_logic seam + ztest, app-local BME280 i2c emulator + ztest
- cleaned up project NOTES

## Known Issue

- consider using clean image to reduce container size
- UBSAN trips inside Zephyr's own BME280 driver (bme280.c:103, left shift of a
  negative value in the Bosch compensation formula). Upstream, not ours; see
  apps/06-sensor/NOTES.md.

## Docs