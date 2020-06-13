// SPDX-License-Identifier: MIT

#include <assert.h>

#include "esp_heap_caps.h"

#include "font_cache.h"

#ifndef FONT_CACHE_ALLOC
#define FONT_CACHE_ALLOC MALLOC_CAP_DEFAULT
#endif

typedef uint32_t font_cache_access_t;
typedef uint16_t font_cache_index_t;
#define FONT_CACHE_ACCESS_MAX UINT32_MAX;


typedef struct font_cache_record {
	font_cache_glyph_t glyph;
	font_cache_index_t index;
	font_cache_access_t access_time;
} font_cache_record_t;


struct font_cache_priv {
	size_t size;
	size_t item_size;
	void *data;
	font_cache_access_t last_access;
	font_cache_record_t *records;
};


esp_err_t font_cache_init(font_cache_t *cache, size_t cache_size, size_t item_size) {
	assert(cache_size < UINT16_MAX);

	cache->priv = (struct font_cache_priv *)heap_caps_malloc(sizeof(struct font_cache_priv), FONT_CACHE_ALLOC);
	if (cache->priv == NULL) {
		return ESP_FAIL;
	}

	cache->priv->records = NULL;
	cache->priv->data = NULL;
	cache->priv->last_access = 0;
	cache->priv->size = cache_size;
	cache->priv->item_size = item_size;

	cache->priv->records = (font_cache_record_t *)heap_caps_malloc(sizeof(font_cache_record_t) * cache_size, FONT_CACHE_ALLOC);
	if (cache->priv->records == NULL) {
		font_cache_destroy(cache);
		return ESP_FAIL;
	}

	cache->priv->data = heap_caps_malloc(item_size * cache_size, FONT_CACHE_ALLOC);
	if (cache->priv->data == NULL) {
		font_cache_destroy(cache);
		return ESP_FAIL;
	}

	for (size_t i = 0; i < cache->priv->size; ++i) {
		cache->priv->records[i].glyph = UINT32_MAX;
		cache->priv->records[i].index = i;
	}

	return ESP_OK;
}


void font_cache_destroy(font_cache_t *cache) {
	if (cache->priv == NULL) {
		return;
	}
	if (cache->priv->records != NULL) {
		heap_caps_free(cache->priv->records);
		cache->priv->records = NULL;
	}
	if (cache->priv->data != NULL) {
		heap_caps_free(cache->priv->data);
		cache->priv->data = NULL;
	}
	heap_caps_free(cache->priv);
	cache->priv = NULL;
}


void *font_cache_get(font_cache_t *cache, font_cache_glyph_t glyph, bool *found) {
	*found = false;
	cache->priv->last_access++;
	font_cache_access_t oldest_access = FONT_CACHE_ACCESS_MAX;
	font_cache_record_t *oldest_record = NULL;

	for (size_t i = 0; i < cache->priv->size; ++i) {
		font_cache_record_t *record = &cache->priv->records[i];
		if (glyph == record->glyph) {
			*found = true;
			void *result = cache->priv->data + cache->priv->item_size * record->index;
			record->access_time = cache->priv->last_access;
			if (oldest_record != NULL) {
				struct font_cache_record tmp = *oldest_record;
				*oldest_record = *record;
				*record = tmp;
			}
			return result;
		}
		else {
			if (record->access_time <= oldest_access) {
				oldest_access = record->access_time;
				oldest_record = record;
			}
		}
	}

	oldest_record->glyph = glyph;
	oldest_record->access_time = cache->priv->last_access;
	return cache->priv->data + cache->priv->item_size * oldest_record->index;
}
