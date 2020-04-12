#pragma once


#include "nanogl.h"


void simulator_display_init(ngl_driver_t *driver, int width, int height, ngl_fb_format_t format, size_t buffer_size);
void simulator_display_destroy(ngl_driver_t *driver);
