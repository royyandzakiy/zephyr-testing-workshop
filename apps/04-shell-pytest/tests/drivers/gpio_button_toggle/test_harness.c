// tests/pytest_shell_gpio/test_harness.c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

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

/* Shell command */
static int cmd_test_button(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    /* Shares the shell's TX path — cannot split the echo of the
     * command that invoked it. */
    shell_print(sh, "Test: Triggering emulated button press");

    trigger_emulated_button_press();
    return 0;
}

SHELL_CMD_REGISTER(test_btn, NULL, "Trigger emulated button press", cmd_test_button);

/* Auto-test after boot (optional) */
#if defined(AUTO_TEST_AFTER_BOOT)
static void auto_test_handler(struct k_work *work)
{
    printk("Auto-test: Triggering button in 5 seconds...\n");
    trigger_emulated_button_press();
}

/* Use a different name for the work object */
static K_WORK_DELAYABLE_DEFINE(auto_test_work_obj, auto_test_handler);

static int test_harness_init(void)
{
    printk("Test Harness initialized\n");
    k_work_schedule(&auto_test_work_obj, K_SECONDS(5));
    return 0;
}

SYS_INIT(test_harness_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
#endif