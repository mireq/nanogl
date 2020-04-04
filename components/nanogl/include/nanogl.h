#pragma once

struct ngl_driver;
struct ngl_buffer;

typedef ngl_buffer *(*ngl_driver_get_buffer_fn) (ngl_driver *driver);
typedef void (*ngl_driver_flush_fn) (ngl_driver *driver);

typedef enum ngl_fb_format {
	RGB_565,
} ngl_fb_format_t;

typedef struct ngl_area {
	int x;
	int y;
	int width;
	int height;
} ngl_area_t;

typedef struct ngl_buffer {
	ngl_area_t area;
	void *buffer;
	struct ngl_driver *driver;
} ngl_buffer;

typedef struct ngl_driver {
	int width;
	int height;
	ngl_fb_format_t format;

	void *user_data;
} ngl_driver_t;


void ngl_init(ngl_driver *driver);
void ngl_destroy(ngl_driver *driver);

/* Writes current buffer to device */
void ngl_flush(ngl_driver *driver);

/* Get next part of buffer in partial mode or secondary buffer in double buffer  mode */
ngl_buffer *ngl_get_buffer(ngl_driver *driver);
