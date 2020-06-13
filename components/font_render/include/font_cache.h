// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"


typedef uint32_t font_cache_glyph_t;


struct font_cache_priv;
typedef struct font_cache {
	struct font_cache_priv *priv;
} font_cache_t;


esp_err_t font_cache_init(font_cache_t *cache, size_t cache_size, size_t item_size);
void font_cache_destroy(font_cache_t *cache);
void *font_cache_get(font_cache_t *cache, font_cache_glyph_t glyph, bool *found);
