// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

#include "esp_err.h"


struct font_face_priv;
struct font_render_priv;

typedef struct font_face {
	struct font_face_priv *priv;
} font_face_t;
typedef struct font_render {
	struct font_render_priv *priv;
} font_render_t;


typedef struct font_area {
	int x;
	int y;
	int width;
	int height;
} font_area_t;

typedef struct font_delta {
	int x;
	int y;
} font_delta_t;

typedef struct font_glyph_metric {
	font_area_t area;
	font_delta_t advance;
} font_glyph_metric_t;


esp_err_t font_face_init(font_face_t *face, const void *data, size_t size);
void font_face_destroy(font_face_t *face);

esp_err_t font_render_init(font_render_t *render, font_face_t *face, unsigned int pixel_size, size_t cache_size);
void font_render_destroy(font_render_t *render);


/*


typedef struct glyph_cache_record {
	uint32_t utf_code;
	uint16_t priority;
	font_size_t bitmap_width;
	font_size_t bitmap_height;
	FT_Glyph_Metrics metrics;
	int bitmap_left;
	int bitmap_top;
	int advance;
} glyph_cache_record_t;

typedef struct font_face {
	FT_Face ft_face;
	font_size_t pixel_size;
} font_face_t;


typedef struct font_render {
	font_face_t *font_face;
	font_size_t max_pixel_width;
	font_size_t max_pixel_height;
	font_size_t origin;
	font_size_t bitmap_width;
	font_size_t bitmap_height;
	font_size_t pixel_size;
	uint16_t cache_size;
	size_t bytes_per_glyph;
	uint8_t *glyph_cache;
	glyph_cache_record_t *glyph_cache_records;
	FT_Glyph_Metrics metrics;
	int bitmap_left;
	int bitmap_top;
	int advance;
	uint8_t *bitmap;
	uint16_t current_priority;
} font_render_t;


esp_err_t font_face_init(font_face_t *face, const font_data_t *data, font_data_size_t size);
void font_face_destroy(font_face_t *face);
esp_err_t font_face_set_pixel_size(font_face_t *face, font_size_t pixel_size);

esp_err_t font_render_init(font_render_t *render, font_face_t *face, font_size_t pixel_size, uint16_t cache_size);
void font_render_destroy(font_render_t *render);
esp_err_t font_load_glyph_metrics(font_render_t *render, uint32_t utf_code);
esp_err_t font_render_glyph(font_render_t *render, uint32_t utf_code);
*/
