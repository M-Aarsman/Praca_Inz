#ifndef _myTriangle
#define _myTriangle

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include <vmath.h>

enum Key {
	KEY_UP,
	KEY_DOWN,
	KEY_NUM
};

class MyTriangle {
public:
	MyTriangle(int height, int width, int majorVersion, int minorVersion,const char title [40]);
	~MyTriangle();
	void run();
	static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	void UpdateCamera();
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

	float _cameraPosX = 0;
	float _cameraPosY = 0;
	float _cameraPosZ = 0;
};

#endif
