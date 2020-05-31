// SPDX-License-Identifier: MIT
#include <assert.h>
#include <sys/param.h>

#include "nanogl.h"


void ngl_event_table_dispatch(ngl_driver_t *driver, ngl_widget_t *widget, ngl_widget_event_table_t *table, ngl_event_t event, void *data) {
	switch (event) {
		case NGL_EVENT_DRAW:
			if (table->draw != NULL) {
				table->draw(driver, widget, (ngl_buffer_t *)data);
			}
			break;
		case NGL_EVENT_INIT:
			if (table->init != NULL) {
				table->init(driver, widget, data);
			}
			break;
		case NGL_EVENT_DESTROY:
			if (table->destroy != NULL) {
				table->destroy(driver, widget);
			}
			break;
		case NGL_EVENT_RESHAPE:
			if (table->reshape != NULL) {
				table->reshape(driver, widget, (ngl_area_t *)data);
			}
			break;
		case NGL_EVENT_FRAME_START:
			if (table->frame_start != NULL) {
				table->frame_start(driver, widget);
			}
			break;
		case NGL_EVENT_FRAME_END:
			if (table->frame_end != NULL) {
				table->frame_end(driver, widget);
			}
			break;
		default:
			if (table->user_event != NULL) {
				table->user_event(driver, widget, event, data);
			}
	}
}


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


void ngl_send_events(ngl_driver_t *driver, ngl_widget_t **widgets, size_t count, ngl_event_t event, void *data) {
	for (size_t i = 0; i < count; ++i) {
		ngl_send_event(driver, widgets[i], event, data);
	}
}


void ngl_widget_init(ngl_driver_t *driver, ngl_widget_t *widget, ngl_widget_process_event_fn process_event, ngl_area_t area, void *widget_priv, void *init_data) {
	widget->process_event = process_event;
	widget->area = area;
	widget->priv = widget_priv;
	ngl_send_event(driver, widget, NGL_EVENT_INIT, init_data);
	ngl_send_event(driver, widget, NGL_EVENT_RESHAPE, &area);
}


void ngl_widget_destroy(ngl_driver_t *driver, ngl_widget_t *widget) {
	ngl_send_event(driver, widget, NGL_EVENT_DESTROY, NULL);
}


void ngl_widget_reshape(ngl_driver_t *driver, ngl_widget_t *widget, ngl_area_t area) {
	ngl_send_event(driver, widget, NGL_EVENT_RESHAPE, &area);
}


void ngl_draw_frame(ngl_driver_t *driver, ngl_widget_t **widgets, size_t count) {
	driver->frame++;

	ngl_send_events(driver, widgets, count, NGL_EVENT_FRAME_START, NULL);

	ngl_buffer_t *buf;
	do {
		buf = ngl_get_buffer(driver);
		ngl_send_events(driver, widgets, count, NGL_EVENT_DRAW, buf);
		ngl_flush(driver);
	} while (buf->area.y + buf->area.height < driver->height);

	ngl_send_events(driver, widgets, count, NGL_EVENT_FRAME_END, NULL);
}


void ngl_fill_area(ngl_buffer_t *target, ngl_area_t *area, ngl_color_t color) {
	assert(target->format == NGL_RGBA);

	// Check draw outside area
	if (
		area->x >= target->area.x + target->area.width ||
		area->y >= target->area.y + target->area.height ||
		area->x + area->width <= target->area.x ||
		area->y + area->height <= target->area.y
	) {
		return;
	}

	ngl_area_t visible_area = {
		.x = MAX(target->area.x, area->x),
		.y = MAX(target->area.y, area->y)
	};
	visible_area.width = MIN(target->area.x + target->area.width, area->x + area->width) - visible_area.x;
	visible_area.height = MIN(target->area.y + target->area.height, area->y + area->height) - visible_area.y;

	ngl_color_t *buffer = (ngl_color_t *)target->buffer;
	size_t target_pos = visible_area.x - target->area.x + (visible_area.y - target->area.y) * target->area.width;
	size_t target_skip = target->area.width - visible_area.width;
	size_t target_end_pos = target_pos + visible_area.width + (visible_area.height - 1) * target->area.width;

	size_t x_pos = 0;

	while (target_pos < target_end_pos) {
		buffer[target_pos].value = color.value;
		x_pos++;
		target_pos++;
		if (x_pos == visible_area.width) {
			target_pos += target_skip;
			x_pos = 0;
		}
	}
}


#include "widgets/rectangle.c"
