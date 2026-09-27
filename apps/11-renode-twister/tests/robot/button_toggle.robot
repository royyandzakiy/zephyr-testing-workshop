*** Settings ***
# ${KEYWORDS} is $ZEPHYR_BASE/tests/robot/common.robot, passed in by Twister. It gives
# `Prepare Machine`, which sets $elf, includes the board's .resc and attaches a terminal
# tester to UART0.
Resource                      ${KEYWORDS}

*** Keywords ***
Boot The DK
    Prepare Machine
    Create LED Tester         sysbus.gpio0.led0
    Wait For Line On Uart     Ready. Press the button to toggle LED.
    Resample The Button Pin

Resample The Button Pin
    # Works around a bug in Renode 1.17's GPIOTE model. When the driver writes a
    # channel's CONFIG register, the model samples the pin, then the OUTINIT field of
    # the same write overwrites that sample with 0. sw0 rests high, so the model thinks
    # it is low, and the first press (high to low) is not seen as an edge. Real hardware
    # ignores OUTINIT in event mode. Writing it as 1 on every event channel puts the
    # stored level back to high without changing anything the firmware can see.
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
