#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "nanogl.h"


void simulator_display_init(ngl_driver_t *driver, int width, int height, ngl_fb_format_t format, size_t buffer_size);
void simulator_display_destroy(ngl_driver_t *driver);
ngl_buffer_t *simulator_display_get_buffer(ngl_driver_t *driver);
void simulator_display_flush(ngl_driver_t *driver);
void simulator_graphic_init(void);
void simulator_graphic_loop(void *data);
void simulator_graphic_process_events(void);
