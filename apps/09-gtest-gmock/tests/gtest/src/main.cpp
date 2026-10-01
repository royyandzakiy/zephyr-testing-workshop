// GoogleTest entry point; unlike ztest, GoogleTest does not generate main().
// No ztest suite or runner: `harness: gtest` in testcase.yaml reads the console
// output and reports one result per test.

#include <zephyr/kernel.h>

#include <gmock/gmock.h>

#if defined(CONFIG_ARCH_POSIX)
// Returning from main() does not end a native_sim run, so the process would idle
// until twister's timeout. nsi_exit() shuts the simulator down; the native_sim
// board already puts nsi_main.h on the include path.
#include <nsi_main.h>
#endif

int main(void)
{
    // GoogleTest parses argv for --gtest_filter and similar. Zephyr's main()
    // takes no arguments, so pass a one-element argv.
    int argc = 1;
    char arg0[] = "zephyr";
    char *argv[] = {arg0, nullptr};

    testing::InitGoogleMock(&argc, argv);

    const int failures = RUN_ALL_TESTS();

#if defined(CONFIG_ARCH_POSIX)
    nsi_exit(failures);
#endif

    return failures;
}
