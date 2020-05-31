#include "nanogl.h"
#include "nanogl/rectangle.h"


static void ngl_widget_rectangle_init(ngl_driver_t *driver, ngl_widget_t *widget, void *data) {
	ngl_color_t *widget_priv = (ngl_widget_rectangle_data_t *)widget->priv;
	ngl_widget_rectangle_init_t *color = (ngl_widget_rectangle_init_t *)data;
	if (color == NULL) {
		widget_priv->value = 0;
	}
	else {
		widget_priv->rgba.r = color->r;
		widget_priv->rgba.g = color->g;
		widget_priv->rgba.b = color->b;
		widget_priv->rgba.a = color->a;
	}
}


static void ngl_widget_rectangle_draw(ngl_driver_t *driver, ngl_widget_t *widget, ngl_buffer_t *buffer) {
	ngl_color_t *color = (ngl_widget_rectangle_data_t *)widget->priv;
	ngl_fill_area(buffer, &widget->area, *color);
}


void ngl_widget_rectangle(ngl_driver_t *driver, ngl_widget_t *widget, ngl_event_t event, void *data) {
	static ngl_widget_event_table_t event_table = {
		.init = ngl_widget_rectangle_init,
		.draw = ngl_widget_rectangle_draw
	};
	ngl_event_table_dispatch(driver, widget, &event_table, event, data);
}
