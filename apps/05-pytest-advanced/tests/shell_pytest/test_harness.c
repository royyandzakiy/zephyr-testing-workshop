// tests/shell_pytest/test_harness.c
//
// The backdoor. Built into the test image only, never into the app.
//
// app 04 had one command, test_btn, and the pytest suite asserted on the
// free-text line the app printed. That is fine for one assertion and awful
// for twenty: the moment someone rewords a printk, every test breaks.
//
// So the commands here print machine-readable key=value lines as well. The
// human line stays, because it is what you want when you are on a serial
// console at 2 AM, but pytest parses the key=value one.

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>

#include "app_state.h"

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* Presses above this are rejected. Not a real limit, just something for a
 * negative test to hit that is not "pass a garbage string". */
#define MAX_PRESSES 20

static inline int button_active_raw(void)
{
    return (button.dt_flags & GPIO_ACTIVE_LOW) ? 0 : 1;
}

void trigger_emulated_button_press(void)
{
    const int active = button_active_raw();
    const int inactive = !active;

    gpio_emul_input_set(button.port, button.pin, inactive);
    k_sleep(K_MSEC(20));

    gpio_emul_input_set(button.port, button.pin, active);
    k_sleep(K_MSEC(50));

    gpio_emul_input_set(button.port, button.pin, inactive);
}

/* app btn [n] -- press the emulated button n times, default 1. */
static int cmd_btn(const struct shell *sh, size_t argc, char **argv)
{
    long n = 1;

    if (argc > 1) {
        char *end;

        n = strtol(argv[1], &end, 10);
        if (*end != '\0' || n < 1 || n > MAX_PRESSES) {
            shell_error(sh, "err=bad_arg arg=%s max=%d", argv[1], MAX_PRESSES);
            return -EINVAL;
        }
    }

    shell_print(sh, "Test: triggering %ld emulated button press(es)", n);

    for (long i = 0; i < n; i++) {
        trigger_emulated_button_press();
    }

    shell_print(sh, "btn=done count=%ld", n);
    return 0;
}

/* app led -- report LED state without touching it. */
static int cmd_led(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "led=%s", app_led_state() ? "on" : "off");
    return 0;
}

/* app stats -- everything a test might want, in one round trip. */
static int cmd_stats(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "presses=%u led=%s uptime_ms=%lld",
                app_press_count(),
                app_led_state() ? "on" : "off",
                k_uptime_get());
    return 0;
}

/* app reset -- zero the counter. The LED is deliberately left alone, so a
 * test that assumes reset also turns the LED off will fail. It is the kind of
 * assumption worth making someone discover rather than warning them about. */
static int cmd_reset(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    app_press_count_reset();
    shell_print(sh, "reset=ok presses=%u", app_press_count());
    return 0;
}

/* app -- list what the backdoor offers.
 *
 * A root command with a subcommand set and a NULL handler makes the shell
 * print its own help, which is fine for a human and a poor thing to assert
 * on: the wording and the return code have both changed between Zephyr
 * releases. Printing the list ourselves costs four lines and makes the test
 * depend on this file rather than on the shell subsystem's formatting. */
static int cmd_app(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "subcmds=btn,led,stats,reset");
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(app_cmds,
    SHELL_CMD_ARG(btn,   NULL, "Press the emulated button [n] times", cmd_btn,   1, 1),
    SHELL_CMD_ARG(led,   NULL, "Print LED state",                     cmd_led,   1, 0),
    SHELL_CMD_ARG(stats, NULL, "Print press count, LED and uptime",   cmd_stats, 1, 0),
    SHELL_CMD_ARG(reset, NULL, "Reset the press counter",             cmd_reset, 1, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(app, &app_cmds, "Test backdoor into the running application", cmd_app);
