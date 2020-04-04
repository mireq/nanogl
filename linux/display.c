#include <stdint.h>
#include <stdlib.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "display.h"


static const char* TAG = "init";


static int argc = 1;
static char *app_name = "simulator";
static char *argv[1];

SemaphoreHandle_t gl_mutex;


typedef struct window_list {
	simulator_window_t *window;
	struct window_list *next;
} window_list_t;

static window_list_t windows = {NULL, NULL};

static void window_register(simulator_window_t *window) {
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
	}
	current->next = (window_list_t *)malloc(sizeof(window_list_t));
	current = current->next;
	current->window = window;
	current->next = NULL;
}

static void window_unregister(simulator_window_t *window) {
	window_list_t *current = &windows;
	window_list_t *prev = &windows;
	while (current->next) {
		prev = current;
		current = current->next;
		if (current->window == window) {
			window_list_t *after = current->next;
			free(current);
			prev->next = after;
			return;
		}
	}
}

simulator_window_t *window_get_by_glut_window(int glut_window) {
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
		if (current->window->glut_window == glut_window) {
			return current->window;
		}
	}
	return NULL;
}


static const GLchar vertex_shader_source[] = "#version 130\n\
\n\
attribute vec2 position;\n\
varying vec2 texcoord;\n\
\n\
void main()\n\
{\n\
	gl_Position = vec4(position, 0.0, 1.0);\n\
	texcoord = position * vec2(0.5) + vec2(0.5);\n\
}";

static const GLchar fragment_shader_source[] = "#version 130\n\
\n\
varying vec2 texcoord;\n\
uniform sampler2D texture;\n\
\n\
void main()\n\
{\n\
	gl_FragColor = texture2D(texture, texcoord);\n\
}";


static GLuint make_buffer(GLenum target, const void *buffer_data, GLsizei buffer_size) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

static void destroy_buffer(GLuint buffer) {
	glDeleteBuffers(1, &buffer);
}

static GLuint make_texture() {
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return texture;
}

static void show_info_log(GLuint object, PFNGLGETSHADERIVPROC get_info_fn, PFNGLGETSHADERINFOLOGPROC get_log_fn)
{
	GLint log_length;
	char *log;

	get_info_fn(object, GL_INFO_LOG_LENGTH, &log_length);
	log = malloc(log_length);
	get_log_fn(object, log_length, NULL, log);
	ESP_LOGE(TAG, "%s", log);
	free(log);
}

static GLuint make_shader(GLenum type, const GLchar *source, GLuint length) {
	GLuint shader;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&source, &length);
	glCompileShader(shader);

	GLint shader_compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);

	if (!shader_compiled) {
		ESP_LOGE(TAG, "Source (size %d)\n%s", length, source);
		ESP_LOGE(TAG, "Failed to compile shader");
		show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static void destroy_shader(GLuint shader) {
	glDeleteShader(shader);
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint shader_linked;
	glGetProgramiv(program, GL_LINK_STATUS, &shader_linked);

	if (!shader_linked) {
		ESP_LOGE(TAG, "Failed to link shader");
		show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

static void destroy_program(GLuint program) {
	glDeleteProgram(program);
}

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

static int make_resources(simulator_window_t *window)
{
	window->vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
	);
	window->element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
	);

	window->texture = make_texture();

	if (window->texture == 0) {
		return 0;
	}

	window->vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		vertex_shader_source,
		sizeof(vertex_shader_source)
	);
	if (window->vertex_shader == 0) {
		return 0;
	}

	window->fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		fragment_shader_source,
		sizeof(fragment_shader_source)
	);

	if (window->fragment_shader == 0) {
		return 0;
	}

	window->program = make_program(window->vertex_shader, window->fragment_shader);
	if (window->program == 0) {
		return 0;
	}

	window->uniforms.texture = glGetUniformLocation(window->program, "texture");
	window->attributes.position = glGetAttribLocation(window->program, "position");

	return 1;
}

static void render(void) {
	simulator_window_t *window = window_get_by_glut_window(glutGetWindow());
	if (window == NULL) {
		return;
	}

	int dirty = window->dirty;
	window->dirty = 0;

	glUseProgram(window->program);

	GLenum format;
	GLenum type;
	switch (window->color_format) {
		case RGB_565:
			format = GL_RGB;
			type = GL_UNSIGNED_SHORT_5_6_5;
			break;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, window->texture);
	if (dirty) {
		glTexImage2D(
			GL_TEXTURE_2D, 0,
			GL_RGB8,
			window->width, window->height, 0,
			format, type,
			window->framebuffer
		);
	}
	glUniform1i(window->uniforms.texture, 0);

	glBindBuffer(GL_ARRAY_BUFFER, window->vertex_buffer);
	glVertexAttribPointer(
		window->attributes.position,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GLfloat)*2,
		(void*)0
	);
	glEnableVertexAttribArray(window->attributes.position);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, window->element_buffer);
	glDrawElements(
		GL_TRIANGLE_STRIP,
		4,
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(window->attributes.position);

	//glFlush();
	glutSwapBuffers();
}


static void destroy_resources(simulator_window_t *window) {
	destroy_program(window->program);
	destroy_shader(window->fragment_shader);
	destroy_shader(window->vertex_shader);
	destroy_buffer(window->element_buffer);
	destroy_buffer(window->vertex_buffer);
}

void simulator_window_flush(simulator_window_t *window) {
	window->dirty = 1;
}

void simulator_graphic_init(void) {
	argv[0] = app_name;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitContextVersion(3, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);

	gl_mutex = xSemaphoreCreateMutex();
	if (gl_mutex == NULL) {
		ESP_LOGE(TAG, "gl_mutex not created");
	}
}

void simulator_graphic_loop(void *data) {
	while (1) {
		simulator_graphic_process_events();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void simulator_graphic_process_events(void) {
	xSemaphoreTake(gl_mutex, portMAX_DELAY);
	glutMainLoopEvent();
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
		glutSetWindow(current->window->glut_window);
		glutPostRedisplay();
	}
	xSemaphoreGive(gl_mutex);
}

void simulator_window_init(simulator_window_t *window, int width, int height, simulator_fb_format format) {
	xSemaphoreTake(gl_mutex, portMAX_DELAY);
	window_register(window);

	window->width = width;
	window->height = height;
	window->color_format = format;
	window->dirty = 1;

	size_t pixel_size;
	switch (format) {
		case RGB_565:
			pixel_size = sizeof(uint16_t);
			break;
	}

	window->framebuffer = malloc(pixel_size * width * height);
	if (window->framebuffer == NULL) {
		ESP_LOGE(TAG, "Framebuffer not allocated");
		window_unregister(window);
		xSemaphoreGive(gl_mutex);
		vTaskDelete(NULL);
		return;
	}

	glutInitWindowSize(width * 2, height * 2);
	window->glut_window = glutCreateWindow("simulator");
	glutDisplayFunc(&render);

	glewExperimental = GL_TRUE;
	glewInit();
	if (!GLEW_VERSION_3_0) {
		ESP_LOGE(TAG, "OpenGL 3.0 not available");
		window_unregister(window);
		xSemaphoreGive(gl_mutex);
		vTaskDelete(NULL);
		return;
	}

	if (!make_resources(window)) {
		ESP_LOGE(TAG, "Failed to load resources");
		window_unregister(window);
		xSemaphoreGive(gl_mutex);
		vTaskDelete(NULL);
		return;
	}

	xSemaphoreGive(gl_mutex);
}


void simulator_window_destroy(simulator_window_t *window) {
	xSemaphoreTake(gl_mutex, portMAX_DELAY);

	if (window->framebuffer != NULL) {
		destroy_resources(window);
		free(window->framebuffer);
		glutDestroyWindow(window->glut_window);
	}

	window_unregister(window);

	xSemaphoreGive(gl_mutex);
}
