#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "gui.h"


#ifndef SIMULATOR
#include "esp_timer.h"
#endif


static long long get_us_time() {
#ifdef SIMULATOR
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long long)ts.tv_sec * 1000000L + ts.tv_nsec / 1000;
#else
	return esp_timer_get_time();
#endif
}


void gui_loop(ngl_driver_t *driver) {
	long frame = 0;
	ngl_buffer_t *buf;
	while (1) {
		frame++;
		uint64_t us_before_frame = get_us_time();
		do {
			buf = ngl_get_buffer(driver);
			ngl_color_t *framebuffer = (ngl_color_t *)buf->buffer;
			for (size_t y = 0; y < buf->area.height; ++y) {
				for (size_t x = 0; x < buf->area.width; ++x) {
					framebuffer[x + y * buf->area.width].rgba.r = (x + buf->area.x)*256/driver->width;
					framebuffer[x + y * buf->area.width].rgba.b = (y + buf->area.y)*256/driver->height;
					framebuffer[x + y * buf->area.width].rgba.g = (framebuffer[x + y * buf->area.width].rgba.r + framebuffer[x + y * buf->area.width].rgba.b) >> 1;
				}
			}
			ngl_flush(driver);
		} while (buf->area.y + buf->area.height < driver->height);
		uint64_t us_after_frame = get_us_time();
		printf("\nf: %08ld, time: %.4f ms", frame, (double)(us_after_frame - us_before_frame) / 1000.);
	}
}
