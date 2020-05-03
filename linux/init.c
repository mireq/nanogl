// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "init.h"


static void gui(void *data) {
	ngl_driver_t driver;

	simulator_display_init(&driver, 240, 240, NGL_RGBA, 240 * 240 * 2);

	for (size_t frame = 0; frame < 100; ++frame) {
		while (1) {
			ngl_buffer_t *buf = ngl_get_buffer(&driver);
			ngl_color_t *framebuffer = (ngl_color_t *)buf->buffer;
			for (size_t y = 0; y < buf->area.height; ++y) {
				for (size_t x = 0; x < buf->area.width; ++x) {
					framebuffer[x + y * buf->area.width].rgba.r = (x + buf->area.x)*256/driver.width;
					framebuffer[x + y * buf->area.width].rgba.b = (y + buf->area.y)*256/driver.height;
					framebuffer[x + y * buf->area.width].rgba.g = (framebuffer[x + y * buf->area.width].rgba.r + framebuffer[x + y * buf->area.width].rgba.b) >> 1;
				}
			}
			ngl_flush(&driver);
			if (buf->area.y + buf->area.height >= driver.height) {
				break;
			}
		}
	}

	simulator_display_destroy(&driver);

	vTaskDelete(NULL);
}

void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}
