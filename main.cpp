#include <stdio.h>
#include <stdlib.h>

#include <pspkernel.h>

#include "psp.hpp"
#include "wa.hpp"

PSP_MODULE_INFO("WA", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

int main() {
    RGBA r = {.channels = {.r = 255, .g = 0, .b = 0, .a = 0}};

    setup_callbacks();
    i32 ok = wa_init();
    MUST(ok);

    while (!exit_request) {
        wa_clear(r);
        wa_swap_bufs();
    }

    sceKernelExitGame();
    return 0;
}

SceUID setup_callbacks() {
    SceUID thid =
        sceKernelCreateThread("wa", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("wa_exit", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int exit_callback(int arg1, int arg2, void *common) {
    exit_request = 1;
    return 0;
}
#pragma GCC diagnostic pop

// See c.h::MUST
void must_cb(const char *expr, const char *file, i32 line) {
    // NOTE: fprintf prints a newline after each argument

    char buf[256];
    snprintf(buf, 256, "Failed MUST %s:%ld: %s\n", file, line, expr);
    fputs(buf, stderr);
    fflush(stderr);

    sceKernelExitGame();
}

// See c.h::ASSERTZ
void assertz_cb(const char *expr, const char *file, i32 line) {
    // NOTE: fprintf prints a newline after each argument

    char buf[256];
    snprintf(buf, 256, "Failed ASSERTZ %s:%ld: %s\n", file, line, expr);
    fputs(buf, stderr);
}
