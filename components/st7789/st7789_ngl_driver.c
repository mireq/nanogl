#include "st7789_ngl_driver.h"
#include "esp_log.h"

#include "freertos/task.h"


static const char *TAG = "st7789_ngl_driver";


typedef struct st7789_ngl_driver_priv {
	ngl_byte_t *framebuffer;
	ngl_buffer_t buffer;
	st7789_driver_t display;
	size_t buffer_size;
	int buffer_lines;
} st7789_ngl_driver_priv_t;



static ngl_buffer_t *st7789_ngl_driver_get_buffer(ngl_driver_t *driver) {
	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)driver->priv;

	driver_priv->buffer.area.y += driver_priv->buffer_lines;
	if (driver_priv->buffer.area.y >= driver->height) {
		driver_priv->buffer.area.y = 0;
	}
	int buffer_height = driver->height - driver_priv->buffer.area.y;
	if (buffer_height > driver_priv->buffer_lines) {
		buffer_height = driver_priv->buffer_lines;
	}
	driver_priv->buffer.area.height = buffer_height;
	return &driver_priv->buffer;
}


static void st7789_ngl_driver_flush(ngl_driver_t *driver) {
	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)driver->priv;
	ngl_color_t *sbuf = (ngl_color_t *)driver_priv->buffer.buffer;
	st7789_color_t *tbuf = driver_priv->display.current_buffer;
	//const st7789_color_t *target_buf_end = target_buf + (driver_priv->buffer.area.width * driver_priv->buffer.area.height);
	const size_t buffer_size = driver_priv->buffer.area.width * driver_priv->buffer.area.height;
	/*
	ngl_color_format_t r, g, b;
	while (target_buf < target_buf_end) {
		r = *src_buf;
		src_buf++;
		g = *src_buf;
		src_buf++;
		b = *src_buf;
		src_buf++;
		*target_buf = r;
		target_buf++;
	}
	*/

	//const uint32_t color = 0xff00ff00;
	//uint32_t *tbuf = (uint32_t *)target_buf;
	//const size_t count = buffer_size >> 1;
	const size_t count = buffer_size;

	/*asm (
		"loop %0, end\n"
		"l32i.n a9, %2, 0\n"
		"s32i.n a8, %1, 0\n"
		"s32i.n a8, %1, 4\n"
		"s32i.n a8, %1, 8\n"
		"s32i.n a8, %1, 12\n"
		"s32i.n a8, %1, 16\n"
		"s32i.n a8, %1, 20\n"
		"s32i.n a8, %1, 24\n"
		"s32i.n a8, %1, 28\n"
		"addi %1, %1, 32\n"
		"addi %2, %2, 64\n"
		"end:"
		: // No outputs
		: "r" (count), "r" (tbuf), "r" (sbuf)
		: "memory", "a8", "a9", "a10", "a11"
	);
	*/


	uint8_t r, g, b;
	for (size_t i = 0; i < count; ++i) {
		r = sbuf[i].rgba.r;
		g = sbuf[i].rgba.g;
		b = sbuf[i].rgba.b;
		tbuf[i] = st7789_rgb_to_color(r, g, b);
	}
	//memset(tbuf, 0, count * 4);

	st7789_swap_buffers(&driver_priv->display);
}


esp_err_t st7789_ngl_driver_init(ngl_driver_t *driver, st7789_ngl_driver_init_struct_t *config) {
	driver->priv = NULL;

	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)malloc(sizeof(st7789_ngl_driver_priv_t));
	if (driver_priv == NULL) {
		ESP_LOGE(TAG, "driver not allocated");
		return ESP_FAIL;
	}

	driver_priv->display.framebuffers = NULL;

	driver->priv = driver_priv;
	driver->flush = st7789_ngl_driver_flush;
	driver->get_buffer = st7789_ngl_driver_get_buffer;
	driver->width = config->width;
	driver->height = config->height;
	driver->format = NGL_RGBA;
	driver_priv->buffer_size = driver->width * config->buffer_lines * 4;
	driver_priv->buffer_lines = config->buffer_lines;
	driver_priv->buffer.area.x = 0;
	driver_priv->buffer.area.y = 0;
	driver_priv->buffer.area.width = driver->width;
	driver_priv->buffer.area.height = driver->height;

	driver_priv->framebuffer = heap_caps_malloc(driver_priv->buffer_size, MALLOC_CAP_DMA);
	if (driver_priv->framebuffer == NULL) {
		ESP_LOGE(TAG, "framebuffer not allocated");
		free(driver->priv);
		driver->priv = NULL;
		return ESP_FAIL;
	}

	driver_priv->buffer.buffer = driver_priv->framebuffer;

	driver_priv->display.pin_reset = config->pin_reset;
	driver_priv->display.pin_dc = config->pin_dc;
	driver_priv->display.pin_mosi = config->pin_mosi;
	driver_priv->display.pin_sclk = config->pin_sclk;
	driver_priv->display.spi_host = config->spi_host;
	driver_priv->display.dma_chan = config->dma_chan;
	driver_priv->display.display_width = config->width;
	driver_priv->display.display_height = config->height;
	driver_priv->display.buffer_size = config->width * config->buffer_lines;
	driver_priv->display.buffer_count = config->buffer_count;

	if (st7789_init(&driver_priv->display) != ESP_OK) {
		free(driver_priv->framebuffer);
		free(driver_priv);
		return ESP_FAIL;
	}

	st7789_reset(&driver_priv->display);
	st7789_lcd_init(&driver_priv->display);
	st7789_wait_until_queue_empty(&driver_priv->display);

	return ESP_OK;
}


esp_err_t st7789_ngl_driver_destroy(ngl_driver_t *driver) {
	st7789_ngl_driver_priv_t *driver_priv = driver->priv;
	if (driver_priv != NULL) {
		if (driver_priv->display.framebuffers != NULL) {
			st7789_wait_until_queue_empty(&driver_priv->display);
			st7789_destroy(&driver_priv->display);
		}
		if (driver_priv->framebuffer != NULL) {
			free(driver_priv->framebuffer);
			driver_priv->framebuffer = NULL;
		}
		free(driver_priv);
	}

	return ESP_OK;
}
