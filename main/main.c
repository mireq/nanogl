// SPDX-License-Identifier: MIT
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init.h"


void app_main(void)
{
	app_init();

	// Clean
	vTaskDelay(portMAX_DELAY);
	vTaskDelete(NULL);
}
