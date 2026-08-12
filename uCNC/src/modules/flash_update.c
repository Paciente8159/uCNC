/*
    Name: flash_update.c
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

#include "../cnc.h"
#include "file_system.h"
#include "net/http.h"
#include "flash_update.h"

#ifdef MCU_HAS_FLASHUPDATE

static flash_udpate_t *flash_upd;

#ifdef ENABLE_SOCKETS

#ifndef OTA_URI
#define OTA_URI "/update"
#endif
static const char updateForm[] __rom__ =
    "<!DOCTYPE html><html><body>"
    "<form method='POST' action='" OTA_URI "' enctype='multipart/form-data'>"
    "Firmware:<br><input type='file' name='firmware'>"
    "<input type='submit' value='Update'>"
    "</form></body></html>";
const char type_html[] = "text/html";
const char type_text[] = "text/plain";

// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
// Request handler for GET /update
static void flash_update_page_cb(int client_idx)
{
    http_send_str(client_idx, 200, (char *)type_html, (char *)updateForm);
    http_send(client_idx, 200, (char *)type_html, NULL, 0);
}

// File upload handler for POST /update
static void flash_update_upload_cb(int client_idx)
{
    http_upload_t up = http_file_upload_status(client_idx);

    if (!flash_upd)
    {
        const char fail[] = "Flash update not available!";
        http_send_str(client_idx, 501, (char *)type_text, (char *)fail);
        http_send(client_idx, 501, (char *)type_text, NULL, 0);
        return;
    }

    if (up.status == HTTP_UPLOAD_START)
    {
        // Called once at start of upload
        proto_printf("Update start: %s\n", up.filename);
        uint32_t maxflash = (!!flash_upd->get_flash_size) ? flash_upd->get_flash_size() : 0;
        if (maxflash < up.filelen)
        {
            // ESP_LOGI("OTA", "File size of %ld exceeds available space: %ld", maxSketchSpace);
            // Update.printError(Serial);
            const char fail[] = "Update Failed: File too big!";
            http_send_str(client_idx, 413, (char *)type_text, (char *)fail);
            http_send(client_idx, 413, (char *)type_text, NULL, 0);
            return;
        }
        if (!flash_upd->flash_begin || !flash_upd->flash_begin(up.filelen))
        {
            const char fail[] = "Update Failed: Update start failed!";
            http_send_str(client_idx, 422, (char *)type_text, (char *)fail);
            http_send(client_idx, 422, (char *)type_text, NULL, 0);
            return;
        }
    }
    else if (up.status == HTTP_UPLOAD_PART)
    {
        // Called for each chunk
        if (!flash_upd->flash_write || (flash_upd->flash_write(up.data, up.datalen) != up.datalen))
        {
            const char fail[] = "Update Failed: Update data error!";
            http_send_str(client_idx, 500, (char *)type_text, (char *)fail);
            http_send(client_idx, 500, (char *)type_text, NULL, 0);
            return;
        }
    }
    else if (up.status == HTTP_UPLOAD_END)
    {
        // Called once at end of upload
        if (!flash_upd->flash_end || flash_upd->flash_end(true))
        {
            const char fail[] = "Update Failed";
            http_send_str(client_idx, 500, (char *)type_text, (char *)fail);
            http_send(client_idx, 500, (char *)type_text, NULL, 0);
        }
        else
        {
            proto_printf("Update Success: %lu bytes\r\n", up.datalen);
            const char suc[] = "Update Success! Rebooting...";
            http_send_str(client_idx, 200, (char *)type_text, (char *)suc);
            http_send(client_idx, 200, (char *)type_text, NULL, 0);
        }

        cnc_delay_ms(100);
        if (flash_upd->device_restart)
        {
            flash_upd->device_restart();
        }
    }
    else if (up.status == HTTP_UPLOAD_ABORT)
    {
        if (flash_upd->flash_end != NULL)
            flash_upd->flash_end(false);
        proto_print("Update aborted\r\n");
    }
}

static char update_uri[] = OTA_URI;

#endif

void flash_update_register(flash_udpate_t *flash_device)
{
    flash_upd = flash_device;
}

DECL_MODULE(flash_update)
{
#ifdef ENABLE_SOCKETS
    LOAD_MODULE(http_server);
    http_add(update_uri, HTTP_REQ_ANY, flash_update_page_cb, flash_update_upload_cb);
#endif
}

#endif
