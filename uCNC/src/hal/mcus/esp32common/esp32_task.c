#include "../../../cnc.h"

#if (ESP32)

#ifdef ENABLE_MAIN_LOOP_MODULES
void esp32_modules_run(void *arg)
{
    while (1)
    {
        if (cnc_check_interlocking())
        {
            modules_dotasks();
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
#endif

void esp32_modules_init(void)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    xTaskCreate(esp32_modules_run, "modulesTask", 16384, NULL, tskIDLE_PRIORITY, NULL);
#endif
}

#endif