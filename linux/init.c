// SPDX-License-Identifier: MIT

#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "gui.h"
#include "init.h"


static void gui(void *data) {
	ngl_driver_t driver;
	/*
	ngl_buffer_t g2;
	g2.area.width = 80;
	g2.area.height = 60;
	g2.area.x = 10;
	g2.area.y = 10;
	g2.buffer = (ngl_byte_t *)(malloc((g2.area.width * g2.area.height >> 2) + ((g2.area.width * g2.area.height) & 0x03 ? 1 : 0)));
	*/

	simulator_display_init(&driver, 240, 240, NGL_RGBA, 240 * 240 * 2);

	/*
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
	*/
	gui_loop(&driver);

	simulator_display_destroy(&driver);

	//free(g2.buffer);

	vTaskDelete(NULL);
}

void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}
