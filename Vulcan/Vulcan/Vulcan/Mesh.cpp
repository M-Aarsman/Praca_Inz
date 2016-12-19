#include "Mesh.h"
#include "VulkanRenderer.h"

#define _USE_MATH_DEFINES
#include <math.h>


Mesh::Mesh(std::vector<Vertex>* vertices, size_t begin, size_t end)
	: _beginIndex(begin),
	  _endIndex(end),
	  _translateX(0),
	  _translateY(0),
	  _translateZ(0),
	  _vertices(vertices) {}

Mesh::~Mesh() {
}

void Mesh::RotateZ(float angle)  {
	//rotation over z axes 3 degree each time
	for(int i = _beginIndex; i < _endIndex; i++) {
		float x = (*_vertices) [i].position.x - _translateX;
		float y = (*_vertices) [i].position.y - _translateY;
		
		(*_vertices) [i].position.x = x * cosf(angle *  M_PI / 180.0) - y * sinf(angle * M_PI / 180.0);
		(*_vertices) [i].position.y = x * sinf(angle *  M_PI / 180.0) + y * cosf(angle * M_PI / 180.0);

		(*_vertices) [i].position.x += _translateX;
		(*_vertices) [i].position.y += _translateY;
	}
}

void Mesh::TranslateX(float n) {
	//muve n over n axe 

	for(int i = _beginIndex; i < _endIndex; i++) {
		float x = (*_vertices) [i].position.x;
		(*_vertices) [i].position.x = x + n;
	}

	_translateX = n;
}

void Mesh::TranslateY(float n) {
	//muve n over y axe 

	for(int i = _beginIndex; i < _endIndex; i++) {
		float y = (*_vertices) [i].position.y;
		(*_vertices) [i].position.y = y + n;
	}

	_translateY = n;
}

void Mesh::TranslateZ(float n) {
	//muve n over z axe 

	for(int i = _beginIndex; i < _endIndex; i++) {
		float z = (*_vertices) [i].position.z;
		(*_vertices) [i].position.z = z + n;
	}

	_translateZ = n;
}
