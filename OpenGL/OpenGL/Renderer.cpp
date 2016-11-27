#include "Renderer.h"
#include <string.h>
#include <assert.h>

bool KEYS [Key::KEY_NUM] = { 0 };

Renderer::Renderer(int height, int width, int majorVersion, int minorVersion, const char title [40])
  : m_windowHeight(height),
	m_windowWidth(width),
	m_majorVersion(majorVersion),
	m_minorVersion(minorVersion)
{
	strcpy(m_title, title);
	init();
}

Renderer::~Renderer() {
	close();
}

void Renderer::run() {
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

void Renderer::init() {
	_cameraPosX = 1.9f;
	_cameraPosY = -0.9f;
	_cameraPosZ = 2.0f;

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
	glfwSetKeyCallback(m_window, OnKey);
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

void Renderer::render(double currentTime) {
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
	UpdateCamera();
}

void Renderer::close() {
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}

void Renderer::loadVertices(char * fileName) {}

void Renderer::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(action == 1) {
		if(key == GLFW_KEY_UP) {
			KEYS [KEY_UP] = true;
		}
		if(key == GLFW_KEY_DOWN) {
			KEYS [KEY_DOWN] = true;
		}
	}
	else if(action == 0) {
		if(key == GLFW_KEY_UP) {
			KEYS [KEY_UP] = false;
		}
		if(key == GLFW_KEY_DOWN) {
			KEYS [KEY_DOWN] = false;
		}
	}

}
void Renderer::UpdateCamera() {
	if(KEYS [KEY_UP]) {
		_cameraPosZ += 0.1;
	} 
	if(KEYS [KEY_DOWN]) {
		_cameraPosZ -= 0.1;
	}

	if(_begin == 0)
		_begin = clock();

	clock_t end = clock();

	static int angle = 0;
	if(double(end - _begin) / CLOCKS_PER_SEC > 0.03) {
		angle = (angle + 3) % 360;
		_begin = end;
	}


	for(int i = 0; i < 30; i++) {
		
		vmath::mat4 mv_matrix = vmath::translate(vmath::vec3(1.0f * i, 0.0f, 0.0f)) *
								vmath::rotate((float)angle, vmath::vec3(0.0f, 0.0f, 1.0f)) *
								vmath::lookat(vmath::vec3(_cameraPosX, _cameraPosY, _cameraPosZ),
											  vmath::vec3(0.0f, 0.0f, 0.0f),
											  vmath::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(m_mv_location, 1, GL_FALSE, mv_matrix);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}
/*void MyTriangle::onResize(int w, int h) {}

void MyTriangle::onKey(int key, int action) {}

void MyTriangle::onMouseButton(int button, int action) {}

void MyTriangle::onMouseMove(double x, double y) {}

void MyTriangle::onMouseWheel(double yoffset) {}*/
