// SPDX-License-Identifier: MIT

#include "esp_log.h"
#include "esp_heap_caps.h"

#include "font_cache.h"
#include "font_render.h"

#include "ft2build.h"
#include FT_FREETYPE_H


static FT_Library ft_library = NULL;
static const char *TAG = "font_render";


#ifndef FONT_ALLOC
#define FONT_ALLOC MALLOC_CAP_DEFAULT
#endif


struct font_face_priv {
	// Freetype font object
	FT_Face ft_face;
	// Current pixel size
	unsigned int pixel_size;
	// Kerning support
	bool has_kerning;
};


struct font_render_priv {
	// Font object
	font_face_t *font;
	// Requested pixel size
	unsigned int pixel_size;
	// Maximum glyph width in pixels
	int max_glyph_width;
	// Maximum glyph height in pixels
	int max_glyph_height;
	// Line height in pixels
	int line_height;
	// Origin position from bottom in pixels
	int origin_position;

	// Cache
	font_cache_t glyph_metric_cache;
};

typedef struct font_glyph_metric {
	// Area required for glyph
	font_area_t area;
	// Advance
	font_delta_t advance;
} font_glyph_metric_t;


esp_err_t font_face_init(font_face_t *face, const void *data, size_t size) {
	face->priv = (struct font_face_priv *)heap_caps_malloc(sizeof(struct font_face_priv), FONT_ALLOC);
	if (face->priv == NULL) {
		return ESP_FAIL;
	}

	FT_Error err;
	face->priv->pixel_size = 0;

	if (ft_library == NULL) {
		err = FT_Init_FreeType(&ft_library);
		if (err) {
			ESP_LOGE(TAG, "Freetype not loaded: %d", err);
			heap_caps_free(face->priv);
			face->priv = NULL;
			return ESP_FAIL;
		}
	}

	err = FT_New_Memory_Face(ft_library, data, size, 0, &face->priv->ft_face);
	if (err) {
		ESP_LOGE(TAG, "Call FT_New_Memory_Face failed: %d", err);
		heap_caps_free(face->priv);
		face->priv = NULL;
		return ESP_FAIL;
	}

	face->priv->has_kerning = FT_HAS_KERNING(face->priv->ft_face);

	return ESP_OK;
}

void font_face_destroy(font_face_t *face) {
	if (face->priv == NULL) {
		return;
	}
	FT_Done_Face(face->priv->ft_face);
	heap_caps_free(face->priv);
	face->priv = NULL;
}

static esp_err_t font_face_set_pixel_size(font_face_t *face, unsigned int pixel_size) {
	if (face->priv->pixel_size != pixel_size) {
		FT_Error err = FT_Set_Pixel_Sizes(face->priv->ft_face, 0, pixel_size);
		if (err) {
			ESP_LOGE(TAG, "Set font size failed: %d", err);
			return ESP_FAIL;
		}
	}
	return ESP_OK;
}

esp_err_t font_render_init(font_render_t *render, font_face_t *face, unsigned int pixel_size, size_t cache_size) {
	render->priv = (struct font_render_priv *)heap_caps_malloc(sizeof(struct font_render_priv), FONT_ALLOC);
	if (render->priv == NULL) {
		return ESP_FAIL;
	}

	if (font_cache_init(&render->priv->glyph_metric_cache, cache_size, sizeof(font_glyph_metric_t)) != ESP_OK) {
		ESP_LOGE(TAG, "Font cache not initialized");
		heap_caps_free(render->priv);
		render->priv = NULL;
		return ESP_FAIL;
	}

	render->priv->font = face;
	render->priv->pixel_size = pixel_size;
	font_face_set_pixel_size(face, render->priv->pixel_size);

	render->priv->max_glyph_width = FT_MulFix((face->priv->ft_face->bbox.xMax - face->priv->ft_face->bbox.xMin), face->priv->ft_face->size->metrics.x_scale) + ((1 << 6) - 1) >> 6;
	render->priv->max_glyph_height = FT_MulFix((face->priv->ft_face->bbox.yMax - face->priv->ft_face->bbox.yMin), face->priv->ft_face->size->metrics.x_scale) + ((1 << 6) - 1) >> 6;
	render->priv->line_height = (face->priv->ft_face->size->metrics.height) >> 6;
	render->priv->origin_position = (-face->priv->ft_face->size->metrics.descender) >> 6;

	return ESP_OK;
}

void font_render_destroy(font_render_t *render) {
	if (render->priv == NULL) {
		return;
	}
	font_cache_destroy(&render->priv->glyph_metric_cache);
	heap_caps_free(render->priv);
	render->priv = NULL;
}

int font_get_line_height(font_render_t *render) {
	return render->priv->line_height;
}

font_glyph_placement_t font_place_glyph(font_render_t *render, font_utf_code_t code, font_pos_t *pos, font_glyph_placement_t *previous) {
	font_glyph_placement_t placement = {
		.area = {0, 0, 0, 0},
		.advance = {0, 0},
		.code.uint = code
	};

	FT_Face face = render->priv->font->priv->ft_face;
	FT_Error err;
	err = FT_Load_Char(face, code, FT_LOAD_RENDER);
	if (err) {
		return placement;
	}

	FT_GlyphSlot slot = face->glyph;

	font_glyph_metric_t metric_;
	font_glyph_metric_t *metric = &metric_;

	metric->area.x = slot->bitmap_left;
	metric->area.y = render->priv->line_height - slot->bitmap_top - render->priv->origin_position;
	metric->area.width = slot->bitmap.width;
	metric->area.height = slot->bitmap.rows;
	metric->advance.x = slot->advance.x >> 6;
	metric->advance.y = slot->advance.y >> 6;

	return placement;
}

/*
static FT_Library ft_library = NULL;
static const char *TAG = "font_render";


#ifndef FONT_CACHE_ALLOC
#define FONT_CACHE_ALLOC MALLOC_CAP_DEFAULT
#endif


static void font_cache_destroy(font_render_t *render) {
	if (render->glyph_cache_records) {
		heap_caps_free(render->glyph_cache_records);
		render->glyph_cache_records = NULL;
	}
	if (render->glyph_cache) {
		heap_caps_free(render->glyph_cache);
		render->glyph_cache = NULL;
	}
}


static esp_err_t font_cache_init(font_render_t *render) {
	font_cache_destroy(render);

	render->max_pixel_width = (render->pixel_size * (render->font_face->ft_face->bbox.xMax - render->font_face->ft_face->bbox.xMin)) / render->font_face->ft_face->units_per_EM + 1;
	render->max_pixel_height = (render->pixel_size * (render->font_face->ft_face->bbox.yMax - render->font_face->ft_face->bbox.yMin)) / render->font_face->ft_face->units_per_EM + 1;
	render->origin = (render->pixel_size * (-render->font_face->ft_face->bbox.yMin)) / render->font_face->ft_face->units_per_EM;
	render->bytes_per_glyph = (size_t)render->max_pixel_width * (size_t)render->max_pixel_height * 2;
	render->bytes_per_glyph = (render->bytes_per_glyph >> 3) + (render->bytes_per_glyph & 0x07 ? 1 : 0);

	render->glyph_cache = (uint8_t *)heap_caps_malloc(render->bytes_per_glyph * render->cache_size, FONT_CACHE_ALLOC);
	render->glyph_cache_records = (glyph_cache_record_t *)heap_caps_malloc(sizeof(glyph_cache_record_t) * render->cache_size, FONT_CACHE_ALLOC);

	if (!render->glyph_cache || !render->glyph_cache_records) {
		ESP_LOGE(TAG, "Glyph cache not allocated");
		if (render->glyph_cache_records) {
			heap_caps_free(render->glyph_cache_records);
			render->glyph_cache_records = NULL;
		}
		if (render->glyph_cache) {
			heap_caps_free(render->glyph_cache);
			render->glyph_cache = NULL;
		}
		return ESP_FAIL;
	}

	memset(render->glyph_cache_records, 0, sizeof(glyph_cache_record_t) * render->cache_size);

	return ESP_OK;
}


esp_err_t font_render_init(font_render_t *render, font_face_t *face, font_size_t pixel_size, uint16_t cache_size) {
	render->font_face = face;
	render->glyph_cache = NULL;
	render->glyph_cache_records = NULL;
	render->pixel_size = pixel_size;
	render->cache_size = cache_size;

	if (font_face_set_pixel_size(face, pixel_size) != ESP_OK) {
		return ESP_FAIL;
	}

	if (font_cache_init(render) != ESP_OK) {
		return ESP_FAIL;
	}

	return ESP_OK;
}


void font_render_destroy(font_render_t *render) {
	font_cache_destroy(render);
}


esp_err_t font_load_glyph_metrics(font_render_t *render, uint32_t utf_code) {
	if (font_face_set_pixel_size(render->font_face, render->pixel_size) != ESP_OK) {
		return ESP_FAIL;
	}

	FT_UInt glyph_index = FT_Get_Char_Index(render->font_face->ft_face, utf_code);
	if (glyph_index == 0) {
		return ESP_FAIL;
	}

	FT_Error err;
	err = FT_Load_Glyph(render->font_face->ft_face, glyph_index,  FT_LOAD_DEFAULT);
	if (err) {
		return ESP_FAIL;
	}

	render->metrics = render->font_face->ft_face->glyph->metrics;

	return ESP_OK;
}


esp_err_t font_render_glyph(font_render_t *render, uint32_t utf_code) {
	// Search cache entry
	int found_cache = -1;
	for (size_t i = 0; i < render->cache_size; ++i) {
		if (render->glyph_cache_records[i].utf_code == utf_code) {
			found_cache = i;
			break;
		}
	}

	if (found_cache == -1) {
		if (font_face_set_pixel_size(render->font_face, render->pixel_size) != ESP_OK) {
			return ESP_FAIL;
		}

		FT_UInt glyph_index = FT_Get_Char_Index(render->font_face->ft_face, utf_code);
		if (glyph_index == 0) {
			return ESP_FAIL;
		}

		FT_Error err;
		err = FT_Load_Glyph(render->font_face->ft_face, glyph_index,  FT_LOAD_DEFAULT);
		if (err) {
			return ESP_FAIL;
		}

		err = FT_Render_Glyph(render->font_face->ft_face->glyph, FT_RENDER_MODE_NORMAL);
		if (err) {
			ESP_LOGE(TAG, "Glyph not rendered %d", err);
			return ESP_FAIL;
		}

		// Find lowest priority
		uint16_t min_priority = 0xffff;
		for (size_t i = 0; i < render->cache_size; ++i) {
			if (render->glyph_cache_records[i].priority <= min_priority) {
				min_priority = render->glyph_cache_records[i].priority;
				found_cache = i;
			}
		}

		render->current_priority++;
		if (render->current_priority == 0) {
			for (size_t i = 0; i < render->cache_size; ++i) {
				render->glyph_cache_records[i].priority = 0;
			}
		}
		render->glyph_cache_records[found_cache].priority = render->current_priority;
		render->glyph_cache_records[found_cache].utf_code = utf_code;
		render->glyph_cache_records[found_cache].metrics = render->font_face->ft_face->glyph->metrics;
		render->glyph_cache_records[found_cache].bitmap_width = render->font_face->ft_face->glyph->bitmap.width;
		render->glyph_cache_records[found_cache].bitmap_height = render->font_face->ft_face->glyph->bitmap.rows;
		render->glyph_cache_records[found_cache].bitmap_left = render->font_face->ft_face->glyph->bitmap_left;
		render->glyph_cache_records[found_cache].bitmap_top = render->font_face->ft_face->glyph->bitmap_top;
		render->glyph_cache_records[found_cache].advance = render->font_face->ft_face->glyph->advance.x >> 6;

		render->bitmap = render->glyph_cache + (render->bytes_per_glyph * found_cache);
		memset(render->bitmap, 0, render->bytes_per_glyph);
		size_t pos = 0;
		for (size_t y = 0; y < render->font_face->ft_face->glyph->bitmap.rows; ++y) {
			for (size_t x = 0; x < render->font_face->ft_face->glyph->bitmap.width; ++x) {
				uint8_t color = render->font_face->ft_face->glyph->bitmap.buffer[y * render->font_face->ft_face->glyph->bitmap.pitch + x];
				render->bitmap[pos >> 2] |= ((color >> 6) << ((pos & 0x03) << 1));
				pos++;
			}
		}
	}


	render->glyph_cache_records[found_cache].priority = render->current_priority;
	render->metrics = render->glyph_cache_records[found_cache].metrics;
	render->bitmap_width = render->glyph_cache_records[found_cache].bitmap_width;
	render->bitmap_height = render->glyph_cache_records[found_cache].bitmap_height;
	render->bitmap_left = render->glyph_cache_records[found_cache].bitmap_left;
	render->bitmap_top = render->glyph_cache_records[found_cache].bitmap_top;
	render->advance = render->glyph_cache_records[found_cache].advance;
	render->bitmap = render->glyph_cache + (render->bytes_per_glyph * found_cache);

	return ESP_OK;
}
*/
