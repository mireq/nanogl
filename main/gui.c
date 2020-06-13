#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "font_render.h"
#include "gui.h"
#include "nanogl/rectangle.h"

#ifndef SIMULATOR
#include "esp_timer.h"
#endif

static const char *TAG = "gui";

static long long get_us_time() {
#ifdef SIMULATOR
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long long)ts.tv_sec * 1000000L + ts.tv_nsec / 1000;
#else
	return esp_timer_get_time();
#endif
}

extern const char *_binary_Ubuntu_R_ttf_start;
extern const size_t Ubuntu_R_ttf_length;

#include "font_cache.h"

void gui_loop(ngl_driver_t *driver) {
	font_face_t ubuntu_font;
	if (font_face_init(&ubuntu_font, _binary_Ubuntu_R_ttf_start, Ubuntu_R_ttf_length) != ESP_OK) {
		ESP_LOGE(TAG, "Font not initialized");
		return;
	}

	font_render_t ubuntu_font_16;
	if (font_render_init(&ubuntu_font_16, &ubuntu_font, 16, 16) != ESP_OK) {
		ESP_LOGE(TAG, "Font render not initialized");
		font_face_destroy(&ubuntu_font);
		return;
	}

	//font_cache_t font_cache;
	//font_cache_init(&font_cache, 4, 1);
	//font_cache_destroy(&font_cache);

	font_render_destroy(&ubuntu_font_16);
	font_face_destroy(&ubuntu_font);


	//printf("%p\n", _binary_Ubuntu_R_ttf_end);

	long frame = 0;
	//ngl_buffer_t *buf;

	ngl_widget_rectangle_data_t rectangle_priv;
	ngl_widget_t rectangle;
	ngl_widget_init(
		driver,
		&rectangle,
		ngl_widget_rectangle,
		&((ngl_area_t){
			.x = 100,
			.y = 100,
			.width = 40,
			.height = 40
		}),
		&rectangle_priv,
		&((ngl_widget_rectangle_init_t){
			.r = 255,
			.g = 255,
			.b = 255,
			.a = 255
		})
	);

	ngl_widget_t *screen[] = {&rectangle};
	while (1) {
		frame++;
		//uint64_t us_before_frame = get_us_time();
		//ngl_draw_frame(driver, screen, sizeof(screen) / sizeof(ngl_widget_t *));
		//bool found;
		//for (size_t i = 0; i < 500; ++i) {
		//	void *data = font_cache_get(&font_cache, i & 0x03, &found);
		//	printf("%ld %p\n", i & 0x03, data);
		//}

		//uint64_t us_after_frame = get_us_time();
		//printf("f: %08ld, time: %.4f ms\n", frame, (double)(us_after_frame - us_before_frame) / 1000.);
	}
}
