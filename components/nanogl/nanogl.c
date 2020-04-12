#include "nanogl.h"


void ngl_flush(ngl_driver_t *driver) {
	driver->flush(driver);
}


ngl_buffer_t *ngl_get_buffer(ngl_driver_t *driver) {
	return driver->get_buffer(driver);
}

unsigned short ngl_get_color_bits(ngl_color_format_t color) {
	switch (color) {
		case NGL_MONO:
			return 1;
		case NGL_GRAY_2:
			return 2;
		case NGL_GRAY_8:
			return 8;
		case NGL_RGB_565:
			return 16;
	}
}


#include "nanogl_widget_rectangle.c"
