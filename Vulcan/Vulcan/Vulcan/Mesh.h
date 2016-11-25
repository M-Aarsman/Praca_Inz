#pragma once
#include "Shared.h"

class Mesh {
public:
	Mesh(std::vector<Vertex>* vertices,size_t begin, size_t end);
	~Mesh();
	
	void RotateZ(float angle);

	void TranslateX(float n);
	void TranslateY(float n);
	void TranslateZ(float n);
private:
	size_t _beginIndex;
	size_t _endIndex;

	int _translateX;
	int _translateY;
	int _translateZ;

	std::vector<Vertex> * _vertices;

public:

};

