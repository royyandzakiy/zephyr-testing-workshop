*** Settings ***
Resource                      ${KEYWORDS}

*** Test Cases ***
Should Boot And Wait For The Button
    Prepare Machine
    Wait For Line On Uart     Ready. Press the button to toggle LED.

Should Turn The LED On At The First Press
    Prepare Machine
    Create LED Tester         sysbus.gpio0.led0
    Wait For Line On Uart     Ready. Press the button to toggle LED.
    Assert LED State          false
    Execute Command           sysbus.gpio0.sw0 PressAndRelease
    Wait For Line On Uart     Button pressed! LED is now ON
    Assert LED State          true

Should Turn The LED Off At The Second Press
    Prepare Machine
    Create LED Tester         sysbus.gpio0.led0
    Wait For Line On Uart     Ready. Press the button to toggle LED.
    Execute Command           sysbus.gpio0.sw0 PressAndRelease
    Wait For Line On Uart     Button pressed! LED is now ON
    Execute Command           sysbus.gpio0.sw0 PressAndRelease
    Wait For Line On Uart     Button pressed! LED is now OFF
    Assert LED State          false
