// SPDX-License-Identifier: MIT

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum ngl_color_format {
	NGL_MONO,
	NGL_GRAY_2,
	NGL_GRAY_8,
	NGL_RGB_565,
	NGL_RGB_888,
} ngl_color_format_t;

typedef enum ngl_event {
	NGL_EVENT_DRAW,
	NGL_EVENT_INIT,
	NGL_EVENT_DESTROY,
	NGL_EVENT_RESHAPE,
	NGL_EVENT_FRAME_START,
	NGL_EVENT_FRAME_END,
} ngl_event_t;

struct ngl_driver;
struct ngl_buffer;
struct ngl_widget;

typedef struct ngl_buffer *(*ngl_driver_get_buffer_fn) (struct ngl_driver *driver);
typedef void (*ngl_driver_flush_fn) (struct ngl_driver *driver);
typedef void (*ngl_widget_process_event_fn) (struct ngl_driver *driver, struct ngl_widget *widget, ngl_event_t event, void *data);
typedef unsigned char ngl_byte_t;

typedef struct ngl_area {
	int x;
	int y;
	int width;
	int height;
} ngl_area_t;

typedef struct ngl_buffer {
	ngl_area_t area;
	ngl_byte_t *buffer;
	struct ngl_driver *driver;
} ngl_buffer_t;

typedef struct ngl_driver {
	int width;
	int height;
	ngl_color_format_t format;

	ngl_driver_get_buffer_fn get_buffer;
	ngl_driver_flush_fn flush;

	void *priv;
} ngl_driver_t;

typedef struct ngl_widget {
	ngl_area_t area;
	ngl_widget_process_event_fn process_event;

	void *priv;
} ngl_widget_t;

typedef union ngl_color {
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} rgba;
	uint32_t value;
} ngl_color_t;


/* Writes current buffer to device */
void ngl_flush(ngl_driver_t *driver);

/* Get next part of buffer in partial mode or secondary buffer in double buffer mode */
ngl_buffer_t *ngl_get_buffer(ngl_driver_t *driver);

/* Get number of bits for each pixel of selected color format */
unsigned short ngl_get_color_bits(ngl_color_format_t color);

/* Send event to widget */
void ngl_send_event(ngl_driver_t *driver, ngl_widget_t *widget, ngl_event_t event, void *data);

/* Send events to widget list */
void ngl_send_events(ngl_driver_t *driver, ngl_widget_t *widgets, size_t count, ngl_event_t event, void *data);

/* Initialize widget */
void ngl_widget_init(ngl_driver_t *driver, ngl_widget_t *widget, ngl_widget_process_event_fn process_events, ngl_area_t area, void *widget_priv);

/* Destroy widget */
void ngl_widget_destroy(ngl_driver_t *driver, ngl_widget_t *widget);

/* Reshape widget */
void ngl_widget_reshape(ngl_driver_t *driver, ngl_widget_t *widget, ngl_area_t area);

/* Draw frame with widgets */
void ngl_draw_frame(ngl_driver_t *driver, ngl_widget_t *widgets);
