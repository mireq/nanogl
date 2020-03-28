#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "init.h"


static void graphic_loop(void *data) {
	while (1) {
		simulator_graphic_process_events();
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

static void gui(void *data) {
	simulator_window_t window;

	simulator_window_init(&window, 240, 240, RGB_565);

	for (size_t i = 0; i < 100; ++i) {
		uint16_t *framebuffer = (uint16_t *)window.framebuffer;
		for (size_t i = 0; i < (window.width * window.height); ++i) {
			framebuffer[i] = rand() % 0xffff;
		}
		simulator_window_flush(&window);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	simulator_window_destroy(&window);

	vTaskDelete(NULL);
}

void app_init(void) {
	simulator_graphic_init();
	xTaskCreate(&graphic_loop, "graphic_loop", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}
