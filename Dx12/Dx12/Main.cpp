#include "Win32Application.h"
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

	Dx12Renderer renderer(50);
	std::wstring title = L"Window";
	Win32Application app(800, 600, title);
	return app.Run(&renderer, hInstance, nCmdShow);
}