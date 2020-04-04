#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "nanogl.h"


typedef struct simulator_window {
	int width;
	int height;
	int glut_window;
	ngl_fb_format_t color_format;
	uint8_t *pixel_buffer_data;
	ngl_buffer_t current_buffer;
	size_t buffer_size;
	int buffer_lines;

	GLuint vertex_buffer, element_buffer, pixel_buffer;
	GLuint texture;
	GLuint vertex_shader, fragment_shader, program;

	struct {
		GLint texture;
	} uniforms;

	struct {
		GLint position;
	} attributes;
} simulator_window_t;


void simulator_window_init(simulator_window_t *window, int width, int height, ngl_fb_format_t format, size_t buffer_size);
void simulator_window_destroy(simulator_window_t *window);
ngl_buffer_t *simulator_get_buffer(simulator_window_t *window);
void simulator_window_flush(simulator_window_t *window);
void simulator_graphic_init(void);
void simulator_graphic_loop(void *data);
void simulator_graphic_process_events(void);
