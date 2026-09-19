> Working notes. Not written for a reader, and parts of it are out of date.
> See [`../README.md`](../README.md) if you are looking for the documentation.

## topics
- emulation
    - on-target: nrf53 blinky with button
    - on-target: nrf53 blinky with button_emul + shell
    - off-target: native_sim with button_emul + shell
- testing
    - running unit tests with ztest
    - running e2e tests pytest
- testing in CI
    - succesful build in CI
    - run unit tests in CI
    - run e2e tests in CI with self-hosted runner

## demo
- on-target: nrf53 blinky with button_emul + shell
- off-target: native_sim with button_emul + shell
- succesful build in CI
- on-target: flash blinky with self-hosted runner

## projects
- basic
    - blinky nrf53 + native_sim
    - shell overlay + test_harness
    - 
- test harness
    - ztest non-twister - basic
        - ztest twister - biz logic
        - twister - hello world
    - pytest non-twister
    - pytest twister - gpio emul shell