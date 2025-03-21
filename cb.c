#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <pspkernel.h>
#pragma GCC diagnostic pop

#include "c.h"

// NOTE: Not its place but w/e
PSP_MODULE_INFO("WA", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

void __printf(const char *what, const char *expr, const char *file, i32 line) {
    // NOTE: fprintf prints a newline after each argument
    char buf[256];
    snprintf(buf, 256, "Failed %s %s:%ld: %s\n", what, file, line, expr);
    fputs(buf, stderr);
}

// See c.h::MUST
void must_cb(const char *expr, const char *file, i32 line) {
    __builtin_trap();
    __printf("MUST", expr, file, line);
    sceKernelExitGame();
}

// See c.h::ASSERTZ
void assert_cb(const char *expr, const char *file, i32 line) {
#ifndef TEST
    __builtin_trap();
#endif
    __printf("ASSERT", expr, file, line);
}

// See compile_tests.py
void testing_started_cb(void) {}

// See compile_tests.py
void test_file_cb(const char *file) { printf("%s\n", file); }

// See compile_tests.py
i32 test_function_cb(i32 (*f)(void), const char *fname) {
    printf("- %s\n", fname);

    i32 passed = (f() == 1) ? 1 : 0;
    if (!passed) {
        printf("-- X %s\n", fname);
    }

    return passed;
}

// See compile_tests.py
void testing_finished_cb(i32 passed, i32 failed) {
    printf("\nPASSED: %ld\n", passed);
    printf("FAILED: %ld\n", failed);
}
