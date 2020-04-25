#pragma once

#include "nanogl.h"
#include "st7789.h"


typedef struct st7789_ngl_driver_init_struct {
	int pin_reset;
	int pin_dc;
	int pin_mosi;
	int pin_sclk;
	int spi_host;
	int dma_chan;
	int width;
	int height;
	int buffer_lines;
	int buffer_count;
} st7789_ngl_driver_init_struct_t;


esp_err_t st7789_ngl_driver_init(ngl_driver_t *driver, st7789_ngl_driver_init_struct_t *config);
esp_err_t st7789_ngl_driver_destroy(ngl_driver_t *driver);
