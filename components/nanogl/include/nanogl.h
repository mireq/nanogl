#pragma once

typedef enum ngl_fb_format {
	NGL_RGB_565,
} ngl_fb_format_t;

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
	ngl_fb_format_t format;

	ngl_driver_get_buffer_fn get_buffer;
	ngl_driver_flush_fn flush;

	void *priv;
} ngl_driver_t;

typedef struct ngl_widget {
	ngl_area_t area;
	ngl_widget_process_event_fn process_event;

	void *priv;
} ngl_widget_t;

typedef struct ngl_widget_list {
	ngl_widget_t *widget;
	struct ngl_widget_list *next;
} ngl_widget_list_t;


/* Writes current buffer to device */
void ngl_flush(ngl_driver_t *driver);

/* Get next part of buffer in partial mode or secondary buffer in double buffer mode */
ngl_buffer_t *ngl_get_buffer(ngl_driver_t *driver);

/* Send event to widget */
void ngl_send_event(ngl_driver_t *driver, ngl_widget_t *widget, ngl_event_t event, void *data);

/* Send events to widget list */
void ngl_send_events(ngl_driver_t *driver, ngl_widget_list_t *widgets, ngl_event_t event, void *data);


