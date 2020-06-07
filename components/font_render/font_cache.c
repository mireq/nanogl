// SPDX-License-Identifier: MIT

#include <assert.h>

#include "esp_heap_caps.h"

#include "font_cache.h"

#ifndef FONT_CACHE_ALLOC
#define FONT_CACHE_ALLOC MALLOC_CAP_DEFAULT
#endif


typedef struct font_cache_record {
	font_cache_glyph_type glyph;
	font_cache_index_type index;
	font_cache_access_type access_time;
} font_cache_record_t;


struct font_cache_priv {
	size_t size;
	font_cache_access_type last_access;
	font_cache_record_t *records;
};


esp_err_t font_cache_init(font_cache_t *cache, size_t cache_size) {
	assert(cache_size < FONT_CACHE_NOT_FOUND);
	cache->priv = (struct font_cache_priv *)heap_caps_malloc(sizeof(struct font_cache_priv), FONT_CACHE_ALLOC);
	if (cache->priv == NULL) {
		return ESP_FAIL;
	}

	cache->priv->records = (font_cache_record_t *)heap_caps_malloc(sizeof(font_cache_record_t) * cache_size, FONT_CACHE_ALLOC);
	if (cache->priv->records == NULL) {
		heap_caps_free(cache->priv);
		cache->priv = NULL;
		return ESP_FAIL;
	}

	return ESP_OK;
}


void font_cache_destroy(font_cache_t *cache) {
	if (cache->priv == NULL) {
		return;
	}
	heap_caps_free(cache->priv->records);
	heap_caps_free(cache->priv);
	cache->priv = NULL;
}
