#include <GL/glew.h>
#include <GL/freeglut.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "init.h"


#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240


static const char* TAG = "init";


static int argc = 1;
static char *app_name = "simulator";
static char *argv[1];

uint16_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];


static struct {
	GLuint vertex_buffer, element_buffer;
	GLuint texture;
	GLuint vertex_shader, fragment_shader, program;

	struct {
		GLint texture;
	} uniforms;

	struct {
		GLint position;
	} attributes;
} g_resources;


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

/*
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer(
	GLenum target,
	const void *buffer_data,
	GLsizei buffer_size
) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

static GLuint make_texture()
{
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

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

static int make_resources(void)
{
	g_resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
	);
	g_resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
	);

	g_resources.texture = make_texture();

	if (g_resources.texture == 0) {
		return 0;
	}

	g_resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		vertex_shader_source,
		sizeof(vertex_shader_source)
	);
	if (g_resources.vertex_shader == 0) {
		return 0;
	}

	g_resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		fragment_shader_source,
		sizeof(fragment_shader_source)
	);
	if (g_resources.fragment_shader == 0) {
		return 0;
	}

	g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);
	if (g_resources.program == 0) {
		return 0;
	}

	g_resources.uniforms.texture = glGetUniformLocation(g_resources.program, "texture");
	g_resources.attributes.position = glGetAttribLocation(g_resources.program, "position");

	return 1;
}


static void render(void) {
	glUseProgram(g_resources.program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_resources.texture);
	glTexImage2D(
		GL_TEXTURE_2D, 0,
		GL_RGB8,
		DISPLAY_WIDTH, DISPLAY_HEIGHT, 0,
		GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,
		framebuffer
	);
	glUniform1i(g_resources.uniforms.texture, 0);

	glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
	glVertexAttribPointer(
		g_resources.attributes.position,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GLfloat)*2,
		(void*)0
	);
	glEnableVertexAttribArray(g_resources.attributes.position);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
	glDrawElements(
		GL_TRIANGLE_STRIP,
		4,
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(g_resources.attributes.position);
	glFlush();
	//glutSwapBuffers();
}


static void simulate_display(void *data) {
	argv[0] = app_name;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitContextVersion(3, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(DISPLAY_WIDTH * 2, DISPLAY_HEIGHT * 2);
	glutCreateWindow("simulator");
	glutDisplayFunc(&render);

	glewExperimental = GL_TRUE;
	glewInit();
	if (!GLEW_VERSION_3_0) {
		ESP_LOGE(TAG, "OpenGL 3.0 not available");
		vTaskDelete(NULL);
	}

	if (!make_resources()) {
		ESP_LOGE(TAG, "Failed to load resources");
		vTaskDelete(NULL);
	}

	while (1) {
		for (size_t i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); ++i) {
			framebuffer[i] = rand() % 0xffff;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
		glutMainLoopEvent();
		glutPostRedisplay();
	}

	vTaskDelete(NULL);
}


void app_init(void) {
	for (size_t i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT); ++i) {
		framebuffer[i] = rand() % 0xffff;
	}
	xTaskCreate(&simulate_display, "display", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

}
