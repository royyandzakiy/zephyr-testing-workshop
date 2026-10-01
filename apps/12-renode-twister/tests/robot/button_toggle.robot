*** Settings ***
# ${KEYWORDS} is $ZEPHYR_BASE/tests/robot/common.robot, passed in by Twister. Its
# `Prepare Machine` sets $elf, includes the board's .resc and attaches a UART0 tester.
Resource                      ${KEYWORDS}

*** Keywords ***
Boot The DK
    Prepare Machine
    Create LED Tester         sysbus.gpio0.led0
    Wait For Line On Uart     Ready. Press the button to toggle LED.
    Resample The Button Pin

Resample The Button Pin
    # Renode 1.17's GPIOTE model lets CONFIG.OUTINIT=0 overwrite the sampled level of the
    # high-resting sw0, so the first press is not an edge. Setting OUTINIT on each event
    # channel restores it; hardware ignores OUTINIT in event mode. See the app's README.md.
    FOR    ${ch}    IN RANGE    8
        ${addr}=    Evaluate    0x40006510 + 4 * ${ch}
        ${cfg}=     Execute Command    sysbus ReadDoubleWord ${addr}
        ${cfg}=     Evaluate    int("${cfg.strip()}", 16)
        IF    ${cfg} & 0x3 == 1
            ${fixed}=    Evaluate    hex(${cfg} | 0x100000)
            Execute Command    sysbus WriteDoubleWord ${addr} ${fixed}
        END
    END

*** Test Cases ***
Should Boot With The LED Off
    Boot The DK
    Assert LED State          false

Should Turn The LED On When sw0 Is Pressed
    Boot The DK
    Execute Command           sysbus.gpio0.sw0 Press
    Wait For Line On Uart     Button pressed! LED is now ON
    Assert LED State          true

Should Toggle On The Press And Not On The Release
    Boot The DK
    Execute Command           sysbus.gpio0.sw0 Press
    Wait For Line On Uart     Button pressed! LED is now ON
    Execute Command           sysbus.gpio0.sw0 Release
    Should Not Be On Uart     Button pressed! LED is now OFF    timeout=1
    Execute Command           sysbus.gpio0.sw0 Press
    Wait For Line On Uart     Button pressed! LED is now OFF
    Assert LED State          false
