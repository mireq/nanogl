// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "init.h"


void gui(void *data) {

	vTaskDelete(NULL);
}


void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE + 2048, NULL, tskIDLE_PRIORITY, NULL);
}
