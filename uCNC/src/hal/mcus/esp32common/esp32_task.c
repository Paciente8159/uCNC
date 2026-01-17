#include "../../../cnc.h"

#if (ESP32)

void esp32_modules_run(void *arg)
{
    while (1)
    {
        EVENT_INVOKE(cnc_dotasks, NULL);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void esp32_modules_init(void)
{
    xTaskCreatePinnedToCore(esp32_modules_run, "modulesTask", 8192, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);
}

#endif