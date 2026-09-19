// tests/gtest/src/main.cpp
//
// The entry point. ztest generates one for you from CONFIG_ZTEST; GoogleTest
// does not, so here it is.
//
// Note what is missing: no CONFIG_ZTEST, no ZTEST_SUITE, no twister-aware
// runner. Twister decides whether this passed by reading the console, which is
// what `harness: console` in testcase.yaml is for. The regex it matches is the
// [ PASSED ] line GoogleTest prints at the end.

#include <zephyr/kernel.h>

#include <gmock/gmock.h>

#if defined(CONFIG_ARCH_POSIX)
// Returning from main() does not end a native_sim run. The kernel carries on
// with the idle thread and the process sits there until something kills it,
// which for twister means waiting out the full timeout on every run. nsi_exit()
// is the native simulator's own shutdown, and the header is already on the
// include path through CONFIG_NATIVE_LIBRARY.
#include <nsi_main.h>
#endif

int main(void)
{
    // GoogleTest parses argv for --gtest_filter and friends. Zephyr's main
    // takes no arguments, so hand it a one-element vector.
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
