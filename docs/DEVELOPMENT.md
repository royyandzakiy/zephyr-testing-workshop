## To Do

- add ubsan to one build option (CI one)

## Done

- created placeholder folders for all apps
- cleaned up project NOTES
- successfully run on nrf53, esp32s3, nucleog4, nativesim
- implement emul based shell test for btn, modify pytest_shell
- prepared dockerfile devcontainer. get and use ncs/vanilla is working. west build, twister is working
- create .github workflow
    - run build samples/blinky
    - build
    - run twister
    - run local runner
- try run with twister
- successfully prepare dockerfile + devcontainer + volume persistent + ncs.py script
- successfully build main

## Known Issue

- fix bad paths caused by changing of folderings

## Docs