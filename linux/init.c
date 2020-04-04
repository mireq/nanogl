#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "display.h"
#include "init.h"


static void gui(void *data) {
	simulator_window_t window;

	simulator_window_init(&window, 240, 240, RGB_565, 240 * 240 * 2);

	for (size_t i = 0; i < 100; ++i) {
		simulator_framebuffer_t *buf = simulator_get_buffer(&window);
		uint16_t *framebuffer = (uint16_t *)buf->buffer;
		for (size_t i = 0; i < (buf->width * buf->height); ++i) {
			framebuffer[i] = rand() % 0xffff;
		}
		simulator_window_flush(&window);
		if (buf->x == 0) {
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
	}

	simulator_window_destroy(&window);

	vTaskDelete(NULL);
}

void app_init(void) {
	simulator_graphic_init();
	xTaskCreate(&simulator_graphic_loop, "graphic_loop", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}
