
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"

int main(void)
{
    xsdk_log_write("J2ME-Loader-360 starting...");

    if (!NT_SUCCESS(xsdk_gpu_open()))
        return 1;

    uint32_t width = 0;
    uint32_t height = 0;

    if (!NT_SUCCESS(xsdk_present_open(
        XSDK_DISPLAY_720P, 2, 1, &width, &height)))
        return 1;

    uint32_t color = 0xFF202060;

    while (true) {
        INPUT_STATE input = {0};

        if (XamInputGetState(0, 0, &input) == ERROR_SUCCESS) {
            if (input.gamepad.buttons & INPUT_GAMEPAD_BACK)
                break;

            if (input.gamepad.buttons & INPUT_GAMEPAD_A)
                color = 0xFF206020;
            else
                color = 0xFF202060;
        }

        uint32_t address;
        if (!NT_SUCCESS(xsdk_present_begin(&address, 5000)))
            break;

        xsdk_gpu_resolve(address, width, height, color);
        xsdk_present_end();
    }

    xsdk_present_close();
    xsdk_gpu_close();

    xsdk_log_write("J2ME-Loader-360 closed.");
    return 0;
}
