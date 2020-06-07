// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

#include "esp_err.h"


typedef uint16_t font_cache_index_type;
typedef uint32_t font_cache_glyph_type;
typedef uint32_t font_cache_access_type;
#define FONT_CACHE_NOT_FOUND UINT16_MAX


struct font_cache_priv;
typedef struct font_cache {
	struct font_cache_priv *priv;
} font_cache_t;


esp_err_t font_cache_init(font_cache_t *cache, size_t cache_size);
void font_cache_destroy(font_cache_t *cache);
