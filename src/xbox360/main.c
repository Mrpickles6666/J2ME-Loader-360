
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/text.h"

#define MAX_GAMES 24
#define GAME_NAME_SIZE 80

static char games[MAX_GAMES][GAME_NAME_SIZE];
static uint32_t game_count = 0;

static uint32_t selected_game = 0;
static bool selection_confirmed = false;
static uint16_t previous_buttons = 0;
static uint16_t current_buttons = 0;

static bool is_jar(const char *name)
{
    const char *ext = strrchr(name, '.');

    if (!ext)
        return false;

    return ext[1] == 'j' || ext[1] == 'J'
        ? (ext[2] == 'a' || ext[2] == 'A')
          && (ext[3] == 'r' || ext[3] == 'R')
          && ext[4] == '\0'
        : false;
}

static void scan_games(void)
{
    game_count = 0;

    DIR *dir = opendir("game:\\");

    if (!dir) {
        xsdk_log_write("J2ME: USB/J2ME folder not found");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        if (!is_jar(entry->d_name))
            continue;

        if (game_count >= MAX_GAMES)
            break;

        snprintf(games[game_count], GAME_NAME_SIZE,
                 "%s", entry->d_name);

        game_count++;
    }

    closedir(dir);

    xsdk_log_write("J2ME: %u JAR files found",
                   (unsigned)game_count);
}

int main(void)
{
    xsdk_log_write("J2ME-Loader-360 starting");

    if (!NT_SUCCESS(xsdk_gpu_open()))
        return 1;

    if (!NT_SUCCESS(xsdk_text_open())) {
        xsdk_gpu_close();
        return 1;
    }

    uint32_t width = 0;
    uint32_t height = 0;

    if (!NT_SUCCESS(xsdk_present_open(
        XSDK_DISPLAY_720P, 2, 1, &width, &height))) {
        xsdk_text_close();
        xsdk_gpu_close();
        return 1;
    }

    scan_games();

    bool running = true;

    while (running) {
        INPUT_STATE input = {0};

        if (XamInputGetState(0, 0, &input) == ERROR_SUCCESS) {
            uint16_t buttons = input.gamepad.buttons;
            current_buttons = buttons;

            if (game_count > 0) {
                if ((buttons & INPUT_GAMEPAD_DPAD_DOWN) &&
                    !(previous_buttons & INPUT_GAMEPAD_DPAD_DOWN)) {
                    if (selected_game < game_count - 1)
                        selected_game++;
                }

                if ((buttons & INPUT_GAMEPAD_DPAD_UP) &&
                    !(previous_buttons & INPUT_GAMEPAD_DPAD_UP)) {
                    if (selected_game > 0)
                        selected_game--;
                }
            }

            if ((buttons & INPUT_GAMEPAD_A) &&
                !(previous_buttons & INPUT_GAMEPAD_A)) {
                if (game_count > 0)
                    selection_confirmed = true;
            }

            if (buttons & INPUT_GAMEPAD_BACK)
                running = false;

            previous_buttons = buttons;
        }

        if (!running)
            break;

        uint32_t address;

        if (!NT_SUCCESS(xsdk_present_begin(&address, 5000)))
            break;

        xsdk_gpu_set_target(width, height);

        xsdk_text_draw(
            40, 35, 2, 0xFFFFFFFF,
            "J2ME-LOADER-360"
        );

        xsdk_text_draw(
            40, 90, 1, 0xFF80FF80,
            "JAVA ME GAME LIBRARY"
        );

        xsdk_text_draw(
            40, 135, 1, 0xFFFFFFFF,
            "USB:/J2ME/"
        );

        char status[80];

        snprintf(status, sizeof(status),
                 "JAR FILES FOUND: %u",
                 (unsigned)game_count);

        xsdk_text_draw(
            40, 175, 1, 0xFFFFFF00, status
        );

        char debug[80];

        snprintf(debug, sizeof(debug),
         "BTN: 0x%04X  A FLAG: %u",
         current_buttons,
         selection_confirmed ? 1 : 0);

        xsdk_text_draw(
            40, 195, 1, 0xFFFFFFFF, debug
        );

        if (selection_confirmed && game_count > 0) {
            char message[100];

            snprintf(message, sizeof(message),
                     "SELECTED: %s", games[selected_game]);

            xsdk_text_draw(
                40, 205, 1, 0xFF80FF80, message
            );
        }

        if (game_count == 0) {
            xsdk_text_draw(
                40, 225, 1, 0xFFFF8080,
                "NO JAR FILES FOUND"
            );
        }

        for (uint32_t i = 0; i < game_count; i++) {
            char line[100];

            snprintf(line, sizeof(line),
                     "%u. %s", (unsigned)(i + 1), games[i]);

            uint32_t color = (i == selected_game)
                ? 0xFFFFFF00
                : 0xFFFFFFFF;

            xsdk_text_draw(
                60, 225 + (int32_t)i * 24,
                1, color, line
            );
        }

        xsdk_text_draw(
            40, height - 45, 1, 0xFFAAAAAA,
            "BACK: EXIT"
        );

        xsdk_gpu_resolve(
            address, width, height, 0xFF101020
        );

        xsdk_present_end();
    }

    xsdk_present_close();
    xsdk_text_close();
    xsdk_gpu_close();

    xsdk_log_write("J2ME-Loader-360 closed");

    return 0;
}
