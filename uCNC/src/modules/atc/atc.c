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

void atc_tool_unmount(uint8_t tool)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    // if no tool about to be mounted
    if (!tool)
    {
        return;
    }

    char tool_unmount_file[] = "/C/atc/toolXXumnt.nc";
    tool_unmount_file[1] = ATC_FS_DRIVE;

    if (tool < 10)
    {
        tool_unmount_file[11] = 0;
        tool_unmount_file[12] = tool;
    }
    else if (tool <= 16)
    {
        tool_unmount_file[11] = 1;
        tool_unmount_file[12] = tool - 10;
    }

    itp_sync();
    // fs_file_run(tool_unmount_file);
    itp_sync();
#endif
}

void atc_tool_mount(uint8_t tool)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    // if no tool about to be mounted
    if (!tool)
    {
        return;
    }

    char tool_mount_file[] = "/C/atc/toolXXmnt.nc";
    tool_mount_file[1] = ATC_FS_DRIVE;

    if (tool < 10)
    {
        tool_mount_file[11] = '0';
        tool_mount_file[12] = '0' + tool;
    }
    else if (tool <= 16)
    {
        tool_mount_file[11] = '1';
        tool_mount_file[12] = '0' + (tool - 10);
    }

    itp_sync();
    // fs_file_run(tool_mount_file);
    itp_sync();
#endif
}

DECL_MODULE(atc)
{
#ifdef ENABLE_ATC_HOOKS
    HOOK_ATTACH_CALLBACK(tool_atc_unmount, atc_tool_unmount);
    HOOK_ATTACH_CALLBACK(tool_atc_mount, atc_tool_mount);
#endif
#ifndef ENABLE_MAIN_LOOP_MODULES
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif
}
