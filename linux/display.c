#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "display.h"


static const char* TAG = "init";


static int argc = 1;
static char *app_name = "simulator";
static char *argv[1];
static GLboolean initialized = GL_FALSE;


SemaphoreHandle_t gl_mutex;
TimerHandle_t process_graphic_events_timer;


typedef struct simulator_window {
	int glut_window;
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


typedef struct window_list {
	ngl_driver_t *driver;
	struct window_list *next;
} window_list_t;

static window_list_t windows = {NULL, NULL};


static size_t get_pixel_size(ngl_fb_format_t format) {
	switch (format) {
		case NGL_RGB_565:
			return sizeof(uint16_t);
	}
	return 0;
}


static GLenum get_gl_pixel_format(ngl_fb_format_t format) {
	switch (format) {
		case NGL_RGB_565:
			return GL_RGB;
		default:
			return 0;
	}
}


static GLenum get_gl_pixel_type(ngl_fb_format_t format) {
	switch (format) {
		case NGL_RGB_565:
			return GL_UNSIGNED_SHORT_5_6_5;
		default:
			return 0;
	}
}


static void window_register(ngl_driver_t *driver) {
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
	}
	current->next = (window_list_t *)malloc(sizeof(window_list_t));
	current = current->next;
	current->driver = driver;
	current->next = NULL;
}

static void window_unregister(ngl_driver_t *driver) {
	window_list_t *current = &windows;
	window_list_t *prev = &windows;
	while (current->next) {
		prev = current;
		current = current->next;
		if (current->driver == driver) {
			window_list_t *after = current->next;
			free(current);
			prev->next = after;
			return;
		}
	}
}

ngl_driver_t *driver_get_by_glut_window(int glut_window) {
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
		if (((simulator_window_t *)current->driver->priv)->glut_window == glut_window) {
			return current->driver;
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
	texcoord = position * vec2(0.5, -0.5) + vec2(0.5);\n\
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


static GLuint make_buffer(GLenum target, const void *buffer_data, GLsizei buffer_size, GLenum usage) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, usage);
	glBindBuffer(target, 0);
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
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

static int make_resources(ngl_driver_t *driver) {
	simulator_window_t *window = (simulator_window_t *)driver->priv;
	window->vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data),
		GL_STATIC_DRAW
	);
	window->element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data),
		GL_STATIC_DRAW
	);
	window->pixel_buffer = make_buffer(
		GL_PIXEL_UNPACK_BUFFER,
		NULL,
		get_pixel_size(driver->format) * driver->width * driver->height,
		GL_STREAM_DRAW
	);

	window->texture = make_texture();

	if (window->texture == 0) {
		return 0;
	}

	const GLenum format = get_gl_pixel_format(driver->format);
	const GLenum type = get_gl_pixel_type(driver->format);
	glBindTexture(GL_TEXTURE_2D, window->texture);
	glTexImage2D(
		GL_TEXTURE_2D, 0,
		GL_RGB8,
		driver->width, driver->height, 0,
		format, type,
		0
	);

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
	ngl_driver_t *driver = driver_get_by_glut_window(glutGetWindow());
	if (driver == NULL) {
		return;
	}
	simulator_window_t *window = (simulator_window_t *)driver->priv;

	glUseProgram(window->program);

	const GLenum format = get_gl_pixel_format(driver->format);
	const GLenum type = get_gl_pixel_type(driver->format);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, window->texture);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, window->pixel_buffer);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0,
		0, 0, driver->width, driver->height,
		format, type,
		0
	);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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


static void destroy_resources(ngl_driver_t *driver) {
	simulator_window_t *window = (simulator_window_t *)driver->priv;
	destroy_program(window->program);
	destroy_shader(window->fragment_shader);
	destroy_shader(window->vertex_shader);
	destroy_buffer(window->element_buffer);
	destroy_buffer(window->vertex_buffer);
}

static ngl_buffer_t *simulator_display_get_buffer(ngl_driver_t *driver) {
	simulator_window_t *window = (simulator_window_t *)driver->priv;

	if (window->pixel_buffer_data == NULL) {
		xSemaphoreTake(gl_mutex, portMAX_DELAY);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, window->pixel_buffer);
		window->pixel_buffer_data = (uint8_t*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		xSemaphoreGive(gl_mutex);
		if (window->pixel_buffer_data == NULL) {
			ESP_LOGE(TAG, "Pixel buffer not mapped");
		}
		window->current_buffer.buffer = window->pixel_buffer_data;
		window->current_buffer.area.y = 0;
		window->current_buffer.area.height = window->buffer_lines;
	}
	else if (window->pixel_buffer_data != NULL) {
		window->current_buffer.area.y += window->buffer_lines;
		size_t pixel_size = get_pixel_size(driver->format);
		size_t buffer_offset = driver->width * window->current_buffer.area.y * pixel_size;
		size_t screen_size = driver->width * driver->height * pixel_size;
		size_t line_size = driver->width * pixel_size;
		window->current_buffer.buffer = window->pixel_buffer_data + buffer_offset;
		int buffer_height = (screen_size - buffer_offset) / line_size;
		if (buffer_height > window->buffer_lines) {
			buffer_height = window->buffer_lines;
		}
		window->current_buffer.area.height = buffer_height;
	}

	return &window->current_buffer;
}

static void simulator_graphic_process_events(TimerHandle_t timer) {
	xSemaphoreTake(gl_mutex, portMAX_DELAY);
	glutMainLoopEvent();
	window_list_t *current = &windows;
	while (current->next) {
		current = current->next;
		glutSetWindow(((simulator_window_t *)current->driver->priv)->glut_window);
		glutPostRedisplay();
	}
	for (size_t i = 0; i < 10; ++i) {
		glutMainLoopEvent();
	}
	xSemaphoreGive(gl_mutex);
}

static void simulator_display_flush(ngl_driver_t *driver) {
	simulator_window_t *window = (simulator_window_t *)driver->priv;
	GLboolean finished = GL_FALSE;

	if (window->current_buffer.area.y + window->buffer_lines >= driver->height) {
		if (window->pixel_buffer_data != NULL) {
			xSemaphoreTake(gl_mutex, portMAX_DELAY);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, window->pixel_buffer);
			xSemaphoreGive(gl_mutex);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			window->pixel_buffer_data = NULL;
			window->current_buffer.buffer = NULL;
		}
		window->current_buffer.area.y = 0;
		finished = GL_TRUE;
	}
	if (finished) {
		simulator_graphic_process_events(process_graphic_events_timer);
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}

static void simulator_graphic_init(void) {
	if (initialized == GL_TRUE) {
		return;
	}

	putenv("vblank_mode=0");

	argv[0] = app_name;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitContextVersion(3, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);

	gl_mutex = xSemaphoreCreateMutex();
	if (gl_mutex == NULL) {
		ESP_LOGE(TAG, "gl_mutex not created");
	}

	process_graphic_events_timer = xTimerCreate("process_graphic_events", 20 / portTICK_PERIOD_MS, pdTRUE, 0, simulator_graphic_process_events);
	if (process_graphic_events_timer == NULL) {
		ESP_LOGE(TAG, "process_graphic_events_timer not allocated");
		return;
	}
	if (xTimerStart(process_graphic_events_timer, 0) != pdPASS) {
		ESP_LOGE(TAG, "process_graphic_events_timer not started");
		return;
	}

	initialized = GL_TRUE;
}


void simulator_display_init(ngl_driver_t *driver, int width, int height, ngl_fb_format_t format, size_t buffer_size) {
	simulator_graphic_init();
	xSemaphoreTake(gl_mutex, portMAX_DELAY);
	driver->priv = malloc(sizeof(simulator_window_t));
	if (driver->priv == NULL) {
		ESP_LOGE(TAG, "Simulator window not allocated");
	}

	simulator_window_t *window = (simulator_window_t *)driver->priv;
	window_register(driver);

	driver->width = width;
	driver->height = height;
	driver->format = format;
	driver->flush = simulator_display_flush;
	driver->get_buffer = simulator_display_get_buffer;

	size_t pixel_size = get_pixel_size(format);
	assert(buffer_size >= width * pixel_size);

	window->buffer_lines = buffer_size / (width * pixel_size);

	window->pixel_buffer_data = NULL;
	window->current_buffer.buffer = NULL;
	window->current_buffer.area.x = 0;
	window->current_buffer.area.y = 0;
	window->current_buffer.area.width = width;
	window->current_buffer.area.height = window->buffer_lines;

	glutInitWindowSize(width * 2, height * 2);
	window->glut_window = glutCreateWindow("simulator");
	glutDisplayFunc(&render);

	glewExperimental = GL_TRUE;
	glewInit();
	if (!GLEW_VERSION_3_0) {
		ESP_LOGE(TAG, "OpenGL 3.0 not available");
		window_unregister(driver);
		xSemaphoreGive(gl_mutex);
		vTaskDelete(NULL);
		return;
	}

	if (!make_resources(driver)) {
		ESP_LOGE(TAG, "Failed to load resources");
		window_unregister(driver);
		xSemaphoreGive(gl_mutex);
		vTaskDelete(NULL);
		return;
	}

	xSemaphoreGive(gl_mutex);
}


void simulator_display_destroy(ngl_driver_t *driver) {
	xSemaphoreTake(gl_mutex, portMAX_DELAY);

	if (driver->priv != NULL) {
		destroy_resources(driver);
		simulator_window_t *window = (simulator_window_t *)driver->priv;
		if (window->pixel_buffer_data != NULL) {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, window->pixel_buffer);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			window->pixel_buffer_data = NULL;
		}

		window_unregister(driver);
		glutDestroyWindow(window->glut_window);
		free(driver->priv);
	}

	xSemaphoreGive(gl_mutex);
}
