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


static uint32_t rng = 0x12345678;


#define st7789_color_pack(c1, c2) ((c1.rgba.r << 24) | (c1.rgba.g << 19) | (c1.rgba.b << 13) | (c2.rgba.r << 8) | (c2.rgba.g << 3) | (c2.rgba.b >> 3))



static void st7789_ngl_driver_flush_simple(ngl_driver_t *driver) {
	static const uint32_t col_mask = 0x00f8fcf8;

	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)driver->priv;
	ngl_color_t *sbuf = (ngl_color_t *)driver_priv->buffer.buffer;
	st7789_color_t *tbuf = driver_priv->display.current_buffer;
	const size_t buffer_size = driver_priv->buffer.area.width * driver_priv->buffer.area.height;
	const size_t count = buffer_size >> 2;

	uint32_t *tptr = (uint32_t *)tbuf;
	ngl_color_t *sptr = sbuf;

	for (size_t i = 0; i < count; ++i) {
		ngl_color_t color1 = sptr[0];
		ngl_color_t color2 = sptr[1];
		color1.value = color1.value & col_mask;
		color2.value = color2.value & col_mask;
		tptr[0] = st7789_color_pack(color1, color2);

		color1 = sptr[2];
		color2 = sptr[3];
		color1.value = color1.value & col_mask;
		color2.value = color2.value & col_mask;
		tptr[1] = st7789_color_pack(color1, color2);

		tptr += 2;
		sptr += 4;
	}
}


static void st7789_ngl_driver_flush_dither(ngl_driver_t *driver) {
	static const uint32_t col_sub_mask = 0x00e000e0;
	static const uint32_t rng_mask = 0x00070307;

	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)driver->priv;
	ngl_color_t *sbuf = (ngl_color_t *)driver_priv->buffer.buffer;
	st7789_color_t *tbuf = driver_priv->display.current_buffer;
	const size_t buffer_size = driver_priv->buffer.area.width * driver_priv->buffer.area.height;
	const size_t count = buffer_size >> 2;

	uint32_t *tptr = (uint32_t *)tbuf;
	ngl_color_t *sptr = sbuf;

	for (size_t i = 0; i < count; ++i) {
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;

		ngl_color_t color1 = sptr[0];
		color1.value -= ((color1.value & col_sub_mask) >> 5);
		color1.rgba.g -= (color1.rgba.g >> 6);
		color1.value += (rng & rng_mask);

		ngl_color_t color2 = sptr[1];
		color2.value -= ((color2.value & col_sub_mask) >> 5);
		color2.rgba.g -= (color2.rgba.g >> 6);
		color2.value += ((rng >> 2) & rng_mask);

		tptr[0] = (st7789_rgb_to_color(color1.rgba.r, color1.rgba.g, color1.rgba.b) << 16) | st7789_rgb_to_color(color2.rgba.r, color2.rgba.g, color2.rgba.b);

		color1 = sptr[2];
		color1.value -= ((color1.value & col_sub_mask) >> 5);
		color1.rgba.g -= (color1.rgba.g >> 6);
		color1.value += ((rng >> 4) & rng_mask);

		color2 = sptr[3];
		color2.value -= ((color2.value & col_sub_mask) >> 5);
		color2.rgba.g -= (color2.rgba.g >> 6);
		color2.value += ((rng >> 6) & rng_mask);

		tptr[1] = (st7789_rgb_to_color(color1.rgba.r, color1.rgba.g, color1.rgba.b) << 16) | st7789_rgb_to_color(color2.rgba.r, color2.rgba.g, color2.rgba.b);

		tptr += 2;
		sptr += 4;
	}
}


static void st7789_ngl_driver_flush(ngl_driver_t *driver) {
	st7789_ngl_driver_priv_t *driver_priv = (st7789_ngl_driver_priv_t *)driver->priv;
	if (driver_priv->display.dither) {
		st7789_ngl_driver_flush_dither(driver);
	}
	else {
		st7789_ngl_driver_flush_simple(driver);
	}
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
	driver_priv->display.dither = true;

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
