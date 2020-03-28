#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>


typedef enum fb_format_ {
	RGB_565,
} fb_format;


typedef struct simulator_window {
	int width;
	int height;
	int glut_window;
	fb_format color_format;
	void *framebuffer;

	GLuint vertex_buffer, element_buffer;
	GLuint texture;
	GLuint vertex_shader, fragment_shader, program;

	struct {
		GLint texture;
	} uniforms;

	struct {
		GLint position;
	} attributes;
} simulator_window_t;


void simulator_window_init(simulator_window_t *window, int width, int height, fb_format format);
void simulator_window_destroy(simulator_window_t *window);
void simulator_graphic_init(void);
void simulator_graphic_process_events(void);