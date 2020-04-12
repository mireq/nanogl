// SPDX-License-Identifier: MIT

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


void ngl_send_event(ngl_driver_t *driver, ngl_widget_t *widget, ngl_event_t event, void *data) {
	widget->process_event(driver, widget, event, data);
}


void ngl_send_events(ngl_driver_t *driver, ngl_widget_t *widgets, size_t count, ngl_event_t event, void *data) {
	for (size_t i = 0; i < count; ++i) {
		ngl_send_event(driver, &widgets[i], event, data);
	}
}


#include "nanogl_widget_rectangle.c"
