#include <stdlib.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "init.h"


static void gui(void *data) {
	ngl_driver_t driver;

	simulator_display_init(&driver, 240, 240, NGL_RGB_888, 240 * 240 * 2);

	for (size_t i = 0; i < 100; ++i) {
		ngl_buffer_t *buf = ngl_get_buffer(&driver);
		uint8_t *framebuffer = (uint8_t *)buf->buffer;
		for (size_t i = 0; i < (buf->area.width * buf->area.height) * 3; ++i) {
			framebuffer[i] = rand() % 0xff;
		}
		ngl_flush(&driver);
	}

	simulator_display_destroy(&driver);

	vTaskDelete(NULL);
}

void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}
