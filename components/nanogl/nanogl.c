#include "nanogl.h"


void ngl_flush(ngl_driver_t *driver) {
	driver->flush(driver);
}


ngl_buffer_t *ngl_get_buffer(ngl_driver_t *driver) {
	return driver->get_buffer(driver);
}


#include "nanogl_widget_rectangle.c"
