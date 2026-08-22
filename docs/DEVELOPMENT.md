## To Do

- create seperate standalone projects
    - consider how is best? their own project? all in one project and called with setting west build target?
- add makefile to compile & run native_sim

## Clean up

- remove pytest.ini & conftest.py (make sure does not affect anything)
- restructure cmake to be target based, such that calling for test harness to compile is more clean and extendable

## Done

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

- fix clangd issue
- fix inconsistency of west twister calls
- remove nrfjprog from dockerfile

## Docs

- NOTES-testing: create explanation of every --flag used
- .devcontainer: explain every part of it, and why decided to do it that way