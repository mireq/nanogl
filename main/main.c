// SPDX-License-Identifier: MIT
#include "esp_log.h"
#include "init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void app_main(void)
{
	app_init();

	// Clean
	vTaskDelay(portMAX_DELAY);
	vTaskDelete(NULL);
}
