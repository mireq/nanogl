// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gui.h"
#include "init.h"
#include "st7789_ngl_driver.h"


#define ST7789_GPIO_RESET GPIO_NUM_19
#define ST7789_GPIO_DC GPIO_NUM_22
#define ST7789_GPIO_MOSI GPIO_NUM_23
#define ST7789_GPIO_SCLK GPIO_NUM_18
#define ST7789_SPI_HOST VSPI_HOST
#define ST7789_DMA_CHAN 2
#define ST7789_DISPLAY_WIDTH 240
#define ST7789_DISPLAY_HEIGHT 240
#define ST7789_BUFFER_SIZE 20


void gui(void *data) {
	ngl_driver_t driver;
	st7789_ngl_driver_init_struct_t ngl_init = {
		.pin_reset=GPIO_NUM_19,
		.pin_dc=GPIO_NUM_22,
		.pin_mosi=GPIO_NUM_23,
		.pin_sclk=GPIO_NUM_18,
		.spi_host=VSPI_HOST,
		.dma_chan=2,
		.width=240,
		.height=240,
		.buffer_lines=20,
		.buffer_count=3,
	};

	ESP_ERROR_CHECK(st7789_ngl_driver_init(&driver, &ngl_init));

	gui_loop(&driver);

	ESP_ERROR_CHECK(st7789_ngl_driver_destroy(&driver));

	vTaskDelete(NULL);
}


void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE + 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
}
