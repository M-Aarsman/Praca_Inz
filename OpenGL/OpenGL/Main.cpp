#include <Windows.h>
#include "MyTriangle.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	static const char title [40] = "OpenGL SuperBible - Simple Clear";
	int winWidth = 800;
	int winHeight = 600;
	int majorVer = 4;
	int minorVer = 5;
	MyTriangle *app = new MyTriangle(winHeight, winWidth, majorVer, minorVer, title);
	app->run();
	delete app;
	return 0;
}