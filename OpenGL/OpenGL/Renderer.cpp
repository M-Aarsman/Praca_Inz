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
	_cameraPosZ = -20.0f;

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
		"in vec4 position;													\n"
		"in vec4 normal;													\n"
		"                                                                   \n"
		"out VS_OUT                                                         \n"
		"{                                                                  \n"
		"    vec4 color;                                                    \n"
		"} vs_out;                                                          \n"
		"                                                                   \n"
		"uniform mat4 m_camera;												\n"
		"uniform mat4 m_proj_matrix;                                        \n"
		"uniform mat4 m_rotate;												\n"
		"uniform mat4 m_translate;											\n"
		"																	\n"
		"uniform vec4 LightPosition;										\n"
		"uniform vec3 Kd;													\n"
		"uniform vec3 Ld;													\n"
		"                                                                   \n"
		"void main(void)                                                    \n"
		"{                                                                  \n"
		"	mat4 model =  m_camera * m_translate * m_rotate	;				\n"
		"    gl_Position = m_proj_matrix * model * position;	\n"
		"    vec4 color = position / 4 + vec4(0.5, 0.5, 0.5, 0.0);			\n"
		"																	\n"
		"																	\n"
		"	vs_out.color =  color ;				\n"
		"	//vs_out.color =  vec4(0.0, 0.0, 0.0, 1.0);						\n"
		"}                                                                  \n"
	};
	//vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);
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

	loadVertices("tedyy.obj");

	_vertexPerMesh = _vertices.size()/3;

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

	m_proj_location = glGetUniformLocation(m_program, "m_proj_matrix");

	m_camera_matrix = vmath::lookat(vmath::vec3(_cameraPosX, _cameraPosY, _cameraPosZ - 10),
				  vmath::vec3(0.0f, 0.0f, 0.0f),
				  vmath::vec3(0.0f, 1.0f, 0.0f));

	m_camera_location = glGetUniformLocation(m_program, "m_camera");
	m_roate_location = glGetUniformLocation(m_program, "m_rotate");
	m_translate_location = glGetUniformLocation(m_program, "m_translate");
	m_translate_matrix = vmath::translate(0.0f, 0.0f, 0.0f);
	m_rotate_matrix = vmath::rotate(1.0f, vmath::vec3(0.0f, 1.0f, 0.0f));
	
		GLuint vboHandles [2];
	glGenBuffers(2, vboHandles);

	GLuint positionBufferHandle = vboHandles [0];
	GLuint normalBufferHandle = vboHandles [1];

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER,
				 _vertices.size() * sizeof(float),
				 _vertices.data(),
				 GL_STATIC_DRAW);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferHandle);
	glBufferData(GL_ARRAY_BUFFER,
				 _normals.size() * sizeof(float),
				 _normals.data(),
				 GL_STATIC_DRAW);
	


	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferHandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	m_aspect = (float) m_windowWidth / (float) m_windowHeight;
	m_proj_matrix = vmath::perspective(50.0f, m_aspect, 0.1f, 1000.0f);


	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glUseProgram(m_program);
	glUniformMatrix4fv(m_camera_location, 1, GL_FALSE, m_camera_matrix);
	glUniformMatrix4fv(m_proj_location, 1, GL_FALSE, m_proj_matrix);
	glUniformMatrix4fv(m_translate_location, 1, GL_FALSE, m_translate_matrix);

	_meshNum = 50;
	unsigned int rowNum = (unsigned int) (sqrt(_meshNum));
	unsigned int meshPerRow = _meshNum / rowNum;

	unsigned int centerIndex = meshPerRow / 2;

	float objectWidth = 6;
	float objectHeight = 4;



	for(int i = 0; i < rowNum; i++) {  // +/- flip
		for(int j = 0; j < meshPerRow; j++) {

			int currentIndex = i*meshPerRow + j;
			if((i*meshPerRow + j) >= _meshNum) {
				break;
			}

			_traslateValueY [currentIndex] = (float) (((int) centerIndex - i) * objectWidth);
			_traslateValueX [currentIndex] = (float) (((int) centerIndex - j) * objectHeight);

		}
	}

	GLint KdLocation = glGetUniformLocation(m_program, "Kd");
	glUniform3fv(KdLocation, 1, vmath::vec3(0.9f, 0.5f, 0.3f));

	GLint LdLocation = glGetUniformLocation(m_program, "Ld");
	glUniform3fv(LdLocation, 1, vmath::vec3(1.0f, 1.0f, 1.0f));

	GLint LightPositionLocation = glGetUniformLocation(m_program, "LightPosition");
	glUniformMatrix4fv(LightPositionLocation, 1, GL_FALSE, m_camera_matrix *  vmath::vec4(0.0f, 0.0f, 0.0f, 20.0f));

}

void Renderer::render(double currentTime) {
	static const GLfloat red [] = { 0.7f, 0.7f, 0.7f, 1.0f };
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_COLOR, 0, red);
	glClearBufferfv(GL_DEPTH, 0, &one);
	
	if(_begin == 0)
		_begin = clock();

	clock_t end = clock();

	static int angle = 0;
	if(double(end - _begin) / CLOCKS_PER_SEC > 0.03) {
		angle = (angle + 2) % 360;
		_begin = end;

		m_rotate_matrix = vmath::rotate((float) angle, vmath::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(m_roate_location, 1, GL_FALSE, m_rotate_matrix);
	}

	for(int i = 0; i < _meshNum; i++) {
		m_translate_matrix = vmath::translate(_traslateValueX [i], _traslateValueY [i], 0.0f);
		glUniformMatrix4fv(m_translate_location, 1, GL_FALSE, m_translate_matrix);
		glDrawArrays(GL_TRIANGLES, 0, _vertexPerMesh);
	}

	UpdateCamera();
}

void Renderer::close() {
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}

void getValuesFromFaceBuffer(char* cBuffer, int values [2]) {
	std::string buffer(cBuffer);

	int endOfFristValue = buffer.find_first_of('/', 0);

	char firtsValue [20] = { 0 };
	for(int i = 0; i < endOfFristValue; i++) {
		firtsValue [i] = buffer [i];
	}

	values [0] = atoi(firtsValue);

	char secondValue [20] = { 0 };
	for(int i = endOfFristValue + 2; buffer [i] != '\0'; i++) {
		secondValue [i - (endOfFristValue + 2)] = buffer [i];
	}

	values [1] = atoi(secondValue);
}

void Renderer::loadVertices(char * fileName) {
	FILE* fp;

	if(!(fp = fopen(fileName, "r"))) {
		assert(0 && "Can not open file!");
	}

	int size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	if(!size) {
		assert(0 && "file is empty!");
	}

	fseek(fp, 0, SEEK_SET);

	struct vertex {
		float x;
		float y;
		float z;
	};

	std::vector<vertex> vertices(0);
	std::vector<vertex> normals(0);
	std::vector<int> faceVectors(0);
	std::vector<float> faceNormals(0);

	char lineHeader [256] = { 0 };

	while(true) {
		int res = fscanf(fp, "%s", lineHeader);
		if(res == EOF) {
			break; // EOF = End Of File. Quit the loop.
		}

		if(!strcmp(lineHeader, "v")) {
			vertex tmp;
			fscanf(fp, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
			vertices.push_back(tmp);
		} else if(!strcmp(lineHeader, "vn")) {
			vertex tmp;
			fscanf(fp, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
			normals.push_back(tmp);
		} else if(!strcmp(lineHeader, "f")) {
			int xn, yn, zn;
			int xv, yv, zv;

			fscanf(fp, "%d//%d %d//%d %d//%d ", &xv, &xn, &yv, &yn, &zv, &zn);
			
			faceVectors.push_back(xv);
			faceVectors.push_back(yv);
			faceVectors.push_back(zv);

			faceNormals.push_back(xn);
			faceNormals.push_back(yn);
			faceNormals.push_back(zn);
		}
	}

	for(int i = 0; i < faceVectors.size(); i++) {
		_vertices.push_back(vertices [faceVectors [i] -1].x);
		_vertices.push_back(vertices [faceVectors [i] -1].y);
		_vertices.push_back(vertices [faceVectors [i] -1].z);
	}

	for(int i = 0; i < faceNormals.size(); i++) {
		_normals.push_back(normals [faceNormals [i] - 1].x);
		_normals.push_back(normals [faceNormals [i] - 1].y);
		_normals.push_back(normals [faceNormals [i] - 1].z);
	}
}

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
	bool changed = false;

	if(KEYS [KEY_UP]) {
		_cameraPosZ += 0.5;
		changed = true;
	} 
	if(KEYS [KEY_DOWN]) {
		_cameraPosZ -= 0.5;
		changed = true;
	}

	if(changed) {
		m_camera_matrix = vmath::lookat(vmath::vec3(_cameraPosX, _cameraPosY, _cameraPosZ - 10),
										vmath::vec3(0.0f, 0.0f, 0.0f),
										vmath::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(m_camera_location, 1, GL_FALSE, m_camera_matrix);
		GLint LightPositionLocation = glGetUniformLocation(m_program, "LightPosition");
		glUniformMatrix4fv(LightPositionLocation, 1, GL_FALSE, m_camera_matrix *vmath::vec4(0.0f, 0.0f, 0.0f, 20.0f));
	}
}
/*void MyTriangle::onResize(int w, int h) {}

void MyTriangle::onKey(int key, int action) {}

void MyTriangle::onMouseButton(int button, int action) {}

void MyTriangle::onMouseMove(double x, double y) {}

void MyTriangle::onMouseWheel(double yoffset) {}*/
