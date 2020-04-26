// SPDX-License-Identifier: MIT
#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7789_ngl_driver.h"
#include "init.h"



#include <string.h>
#include "soc/cpu.h"


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

	for (size_t i = 0; i < 1000; ++i) {
		uint64_t ticks_before_frame = esp_cpu_get_ccount();
		for (size_t j = 0; j < 12; ++j) {
			//uint8_t val = rand() % 0xff;
			//uint8_t val = i % 256;
			//uint8_t val = i % 256;
			ngl_buffer_t *buf = ngl_get_buffer(&driver);
			ngl_color_t *framebuffer = (ngl_color_t *)buf->buffer;
			//size_t bytes_count = (buf->area.width * buf->area.height) * 4;
			/*
			*/
			if (i == 0 && j == 0) {
				//memset(framebuffer, val, bytes_count);
				for (size_t y = 0; y < buf->area.height; ++y) {
					for (size_t x = 0; x < buf->area.width; ++x) {
						framebuffer[x + y * buf->area.width].rgba.r = x*256/buf->area.width;
						framebuffer[x + y * buf->area.width].rgba.g = x*256/buf->area.width;
						framebuffer[x + y * buf->area.width].rgba.b = x*256/buf->area.width;
					}
				}
			}
			ngl_flush(&driver);
		}
		uint64_t ticks_after_frame = esp_cpu_get_ccount();
		printf("\nf: %08d, time: %.4f", (int)i, ((double)ticks_after_frame - (double)ticks_before_frame) / 240000.0);
	}

	ESP_ERROR_CHECK(st7789_ngl_driver_destroy(&driver));

	vTaskDelete(NULL);
}


void app_init(void) {
	xTaskCreate(&gui, "gui", configMINIMAL_STACK_SIZE + 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
}
