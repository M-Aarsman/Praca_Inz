#ifndef _renderer
#define _renderer

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "Share.h"
#include <vector>
#include <time.h>

class Renderer {
public:
	Renderer(int height, int width, int majorVersion, int minorVersion,const char title [40]);
	~Renderer();
	void run();
	static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	void UpdateCamera();
private:
	void init();
	void render(double currentTime);
	void close();

private:
	void loadVertices(char* fileName);
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
	GLint  m_camera_location;
	GLint  m_roate_location;
	GLint  m_translate_location;

	vmath::mat4 m_proj_matrix;
	vmath::mat4 m_camera_matrix;
	vmath::mat4 m_rotate_matrix;
	vmath::mat4 m_translate_matrix;

	float m_aspect;

	float _cameraPosX = 0;
	float _cameraPosY = 0;
	float _cameraPosZ = 0;

	float _traslateValueX [18000] = { 0 };
	float _traslateValueY [18000] = { 0 };

	std::vector<Vertex> vertices;

	unsigned int _vertexPerMesh = 0;
	unsigned int _meshNum;

	clock_t _begin = 0;
};

#endif
