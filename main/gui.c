#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "gui.h"


static const char *TAG = "gui";
#ifndef SIMULATOR
#include "esp32/clk.h"
static long ccounter_overflow_count = 0;
static esp_cpu_ccount_t last_ccount = 0;
#endif


static long long get_ns_time() {
#ifdef SIMULATOR
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
#else
	esp_cpu_ccount_t ccount = esp_cpu_get_ccount();
	if (ccount < last_ccount) {
		ccounter_overflow_count++;
	}
	last_ccount = ccount;
	// Get cycle count normalized to 240 MHz
	int normalize = 4 - (esp_clk_cpu_freq() / 80000000);
	long long total_cycle = (((long long)1 << (sizeof(esp_cpu_ccount_t) * 8)) * ccounter_overflow_count + last_ccount) * normalize;
	return total_cycle * 25 / 2;
#endif
}


static void ns_overflow_check_callback(TimerHandle_t timer) {
	get_ns_time();
}


void gui_loop(ngl_driver_t *driver) {
	TimerHandle_t get_ns_timer = xTimerCreate(
		"ns_overflow_check",
		1000 / portTICK_PERIOD_MS,
		pdTRUE,
		(void *)0,
		ns_overflow_check_callback
	);
	if (get_ns_timer == NULL) {
		ESP_LOGE(TAG, "Timer ns_overflow_check not created");
		return;
	}
	if (xTimerStart(get_ns_timer, 0) != pdPASS) {
		ESP_LOGE(TAG, "Timer ns_overflow_check not started");
		xTimerDelete(get_ns_timer, 0);
		return;
	}

	long frame = 0;
	ngl_buffer_t *buf;
	while (1) {
		frame++;
		//uint64_t ticks_before_frame = esp_cpu_get_ccount();
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
		//uint64_t ticks_after_frame = esp_cpu_get_ccount();
		//printf("\nf: %08d, time: %.4f", (int)frame, ((double)ticks_after_frame - (double)ticks_before_frame) / 240000.0);
		printf("\nf: %08ld, time: %f", frame, (double)(get_ns_time()));
	}

	xTimerStop(get_ns_timer, 0);
	xTimerDelete(get_ns_timer, 0);
}
