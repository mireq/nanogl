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
		case NGL_RGB_888:
			return 24;
		case NGL_RGBA:
			return 32;
	}
	return 0;
}


void ngl_send_event(ngl_driver_t *driver, ngl_widget_t *widget, ngl_event_t event, void *data) {
	widget->process_event(driver, widget, event, data);
}


void ngl_send_events(ngl_driver_t *driver, ngl_widget_t *widgets, size_t count, ngl_event_t event, void *data) {
	for (size_t i = 0; i < count; ++i) {
		ngl_send_event(driver, &widgets[i], event, data);
	}
}


void ngl_widget_init(ngl_driver_t *driver, ngl_widget_t *widget, ngl_widget_process_event_fn process_events, ngl_area_t area, void *widget_priv, void *init_data) {
	ngl_send_event(driver, widget, NGL_EVENT_INIT, init_data);
	ngl_send_event(driver, widget, NGL_EVENT_RESHAPE, &area);
}


void ngl_draw_pixmap(ngl_buffer_t *target, ngl_buffer_t *source, ngl_area_t *crop, ngl_color_t color) {
}


#include "nanogl_widget_rectangle.c"
