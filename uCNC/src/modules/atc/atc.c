/*
    ATC - automatic tool changer module
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>
#include "../file_system.h"

#ifndef ATC_FS_DRIVE
#define ATC_FS_DRIVE 'C'
#endif

#if (UCNC_MODULE_VERSION < 11500 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

static fs_file_t *atc_file;
static grbl_stream_t *prev_stream;
static bool atc_running;

static void atc_close()
{
    atc_running = false;
    fs_close(atc_file);
    atc_file = NULL;
    grbl_stream_change(prev_stream); // restores the previous stream
}

static uint8_t atc_getc(void)
{
    if (atc_file)
    {
        if (fs_available(atc_file))
        {
            uint8_t c = 0;
            fs_read(atc_file, &c, 1);
            return c;
        }
    }
    atc_close();
    return EOL;                      // line brake to escape the ok
}

DECL_GRBL_STREAM(atc_stream, atc_getc, NULL, NULL, NULL, NULL);

static void atc_open(uint8_t index, bool mount, uint8_t *status)
{
    if (atc_file)
    {
        fs_close(atc_file);
        atc_file = NULL;
    }

    char atc_filename[32];
    memset(atc_filename, 0, sizeof(atc_filename));
    if (mount)
    {
        str_sprintf(atc_filename, "/%c/atc/tool%dmnt.nc", ATC_FS_DRIVE, index);
    }
    else
    {
        str_sprintf(atc_filename, "/%c/atc/tool%dumnt.nc", ATC_FS_DRIVE, index);
    }
    atc_file = fs_open(atc_filename, "r");
    if (!atc_file)
    {
        return;
    }

#ifdef ENABLE_ATC_VERBOSE
    prev_stream = grbl_stream_readonly(atc_getc, NULL, NULL);
#else
    prev_stream = grbl_stream_change(&atc_stream);
#endif
    atc_running = true;
    do
    {
        uint8_t error = cnc_parse_cmd();
        if (error)
        {
            // a gcode error happened - leave
            *status = error;
            atc_close();
            return;
        }
    } while (cnc_dotasks() && atc_running);
}

void atc_tool_unmount(uint8_t tool, uint8_t *status)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    // if no tool about to be mounted
    if (!tool)
    {
        return;
    }

    atc_open(tool, false, status);
#endif
}

void atc_tool_mount(uint8_t tool, uint8_t *status)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    // if no tool about to be mounted
    if (!tool)
    {
        return;
    }

    atc_open(tool, true, status);
#endif
}

DECL_MODULE(atc)
{
#if defined(ENABLE_ATC_HOOKS) && (TOOL_COUNT > 1)
    HOOK_ATTACH_CALLBACK(tool_atc_unmount, atc_tool_unmount);
    HOOK_ATTACH_CALLBACK(tool_atc_mount, atc_tool_mount);
#endif
#ifndef ENABLE_MAIN_LOOP_MODULES
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif
}
