#pragma once

struct ngl_driver;
struct ngl_buffer;

typedef struct ngl_buffer *(*ngl_driver_get_buffer_fn) (struct ngl_driver *driver);
typedef void (*ngl_driver_flush_fn) (struct ngl_driver *driver);
typedef unsigned char ngl_byte_t;

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


/* Writes current buffer to device */
void ngl_flush(ngl_driver_t *driver);

/* Get next part of buffer in partial mode or secondary buffer in double buffer mode */
ngl_buffer_t *ngl_get_buffer(ngl_driver_t *driver);
