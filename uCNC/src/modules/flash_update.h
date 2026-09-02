/*
    Name: flash_update.h
    Description: Flash updater for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 12-08-2026

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef FLASH_UPDATE_H
#define FLASH_UPDATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

    typedef struct flash_udpate_
    {
        /* Gets the maximum allowable size of the flash upadte file */
        size_t (*get_flash_size)(void);
        /* Starts the flash update process. Returns false if fails*/
        bool (*flash_begin)(size_t filesize);
        /*writes a chunk of data. returns the amount of bytes writen*/
        size_t (*flash_write)(uint8_t *data, size_t len);
        /*ends the flashing process. returns true if sucessfull*/
        bool (*flash_end)(bool flush);
        void (*device_restart)(void);
    } flash_udpate_t;

    void flash_update_register(flash_udpate_t *flash_device);
    DECL_MODULE(flash_update);

#ifdef __cplusplus
}
#endif
#endif
