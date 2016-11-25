#include "MyTriangle.h"
#include <string.h>
#include <assert.h>

MyTriangle::MyTriangle(int height, int width, int majorVersion, int minorVersion, const char title [40])
  : m_windowHeight(height),
	m_windowWidth(width),
	m_majorVersion(majorVersion),
	m_minorVersion(minorVersion)
{
	strcpy(m_title, title);
	init();
}

MyTriangle::~MyTriangle() {
	close();
}

void MyTriangle::run() {
	bool running = true;

	do {
		render(glfwGetTime());

		glfwSwapBuffers(m_window);
		glfwPollEvents();

		running &= (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
		running &= (glfwWindowShouldClose(m_window) != GL_TRUE);
	} while(running);

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void MyTriangle::init() {
	if(!glfwInit()) {
		assert(0 && "Failed to initialize GLFW\n");
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_majorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_minorVersion);

	{
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	{
		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_title, NULL, NULL);
		if(!m_window) {
			assert(0 && "Failed to open window\n");
			return;
		}
	}

	glfwMakeContextCurrent(m_window);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	gl3wInit();

	static const char * vs_source [] =
	{
		"#version 410 core                                                  \n"
		"                                                                   \n"
		"in vec4 position;                                                  \n"
		"                                                                   \n"
		"out VS_OUT                                                         \n"
		"{                                                                  \n"
		"    vec4 color;                                                    \n"
		"} vs_out;                                                          \n"
		"                                                                   \n"
		"uniform mat4 m_mv_matrix;                                          \n"
		"uniform mat4 m_proj_matrix;                                        \n"
		"                                                                   \n"
		"void main(void)                                                    \n"
		"{                                                                  \n"
		"    gl_Position = m_proj_matrix * m_mv_matrix * position;			\n"
		"    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
		"}                                                                  \n"
	};

	static const char * fs_source [] =
	{
		"#version 410 core                                                  \n"
		"                                                                   \n"
		"out vec4 color;                                                    \n"
		"                                                                   \n"
		"in VS_OUT                                                          \n"
		"{                                                                  \n"
		"    vec4 color;                                                    \n"
		"} fs_in;                                                           \n"
		"                                                                   \n"
		"void main(void)                                                    \n"
		"{                                                                  \n"
		"    color = fs_in.color;                                           \n"
		"}                                                                  \n"
	};

	static const GLfloat vertex_positions [] =
	{
		-0.25f,  0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		0.25f, -0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		0.25f,  0.25f, -0.25f,
		-0.25f,  0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		0.25f, -0.25f,  0.25f,
		0.25f,  0.25f, -0.25f,

		0.25f, -0.25f,  0.25f,
		0.25f,  0.25f,  0.25f,
		0.25f,  0.25f, -0.25f,

		0.25f, -0.25f,  0.25f,
		-0.25f, -0.25f,  0.25f,
		0.25f,  0.25f,  0.25f,

		-0.25f, -0.25f,  0.25f,
		-0.25f,  0.25f,  0.25f,
		0.25f,  0.25f,  0.25f,

		-0.25f, -0.25f,  0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f,  0.25f,  0.25f,

		-0.25f, -0.25f, -0.25f,
		-0.25f,  0.25f, -0.25f,
		-0.25f,  0.25f,  0.25f,

		-0.25f, -0.25f,  0.25f,
		0.25f, -0.25f,  0.25f,
		0.25f, -0.25f, -0.25f,

		0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f,  0.25f,

		-0.25f,  0.25f, -0.25f,
		0.25f,  0.25f, -0.25f,
		0.25f,  0.25f,  0.25f,

		0.25f,  0.25f,  0.25f,
		-0.25f,  0.25f,  0.25f,
		-0.25f,  0.25f, -0.25f
	};

	m_program = glCreateProgram();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, fs_source, NULL);
	glCompileShader(fs);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, vs_source, NULL);
	glCompileShader(vs);

	glAttachShader(m_program, vs);
	glAttachShader(m_program, fs);

	glLinkProgram(m_program);

	m_mv_location = glGetUniformLocation(m_program, "m_mv_matrix");
	m_proj_location = glGetUniformLocation(m_program, "m_proj_matrix");

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(vertex_positions),
				 vertex_positions,
				 GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void MyTriangle::render(double currentTime) {
	static const GLfloat red [] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static const GLfloat one = 1.0f;
	m_aspect = (float) m_windowWidth / (float) m_windowHeight;
	m_proj_matrix = vmath::perspective(50.0f, m_aspect, 0.1f, 1000.0f);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glClearBufferfv(GL_COLOR, 0, red);
	glClearBufferfv(GL_DEPTH, 0, &one);
	glUseProgram(m_program);
	glUniformMatrix4fv(m_proj_location, 1, GL_FALSE, m_proj_matrix);
	//float f = (float) currentTime * 0.3f;
	vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(1.9f, -0.9f, 4.0f),
										  vmath::vec3(0.0f, 0.0f, 0.0f),
										  vmath::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(m_mv_location, 1, GL_FALSE, mv_matrix);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void MyTriangle::close() {
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}

/*void MyTriangle::onResize(int w, int h) {}

void MyTriangle::onKey(int key, int action) {}

void MyTriangle::onMouseButton(int button, int action) {}

void MyTriangle::onMouseMove(double x, double y) {}

void MyTriangle::onMouseWheel(double yoffset) {}*/
