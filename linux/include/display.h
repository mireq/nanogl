#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>


typedef enum simulator_fb_format_ {
	RGB_565,
} simulator_fb_format;


typedef struct simulator_framebuffer {
	uint8_t *buffer;
	int x;
	int y;
	int width;
	int height;
} simulator_framebuffer_t;


typedef struct simulator_window {
	int width;
	int height;
	int glut_window;
	simulator_fb_format color_format;
	uint8_t *pixel_buffer_data;
	simulator_framebuffer_t current_buffer;
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


void simulator_window_init(simulator_window_t *window, int width, int height, simulator_fb_format format, size_t buffer_size);
void simulator_window_destroy(simulator_window_t *window);
simulator_framebuffer_t *simulator_get_buffer(simulator_window_t *window);
void simulator_window_flush(simulator_window_t *window);
void simulator_graphic_init(void);
void simulator_graphic_loop(void *data);
void simulator_graphic_process_events(void);
