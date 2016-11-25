#pragma once

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include <vmath.h>

class MyTriangle {
public:
	MyTriangle(int height, int width, int majorVersion, int minorVersion,const char title [40]);
	~MyTriangle();
	void run();
private:
	void init();
	void render(double currentTime);
	void close();

private:
	int m_windowHeight;
	int m_windowWidth;
	char m_title [128];
	int m_majorVersion;
	int m_minorVersion;
	GLFWwindow* m_window;
	GLuint m_program;
	GLuint m_vao;
	GLuint m_buffer;
	GLint  m_proj_location;
	vmath::mat4 m_proj_matrix;
	GLint  m_mv_location;
	vmath::mat4 m_mv_matrix;
	float m_aspect;
};

