#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanWindow.h"

#include <assert.h>
#include <iostream>

bool KEYS [Key::KEY_NUM] = { 0 };

LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch(uMsg) {
		case WM_CLOSE:
			window->Close();
		break;;
		
		case WM_SIZE:
		break;
		default:
		break;
		
		case WM_KEYDOWN: {
			switch(wParam) {
				case VK_ESCAPE:
					window->Close();
				break;
				case VK_UP:
					KEYS [KEY_UP] = true;
				break;
				case VK_DOWN:
					KEYS [KEY_DOWN] = true;
				break;
			}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Window::Window(VulkanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, const char* name) {
	assert(renderer != nullptr, "Renderer is nullptr!");

	_win32ClassName = "ApiWindow";
	_win32Instance = GetModuleHandle(nullptr);
	_sizeX = sizeX;
	_sizeY = sizeY;
	_winTitle = name;

	initOSWindow();

	_renderer = renderer;
	_vulkanWindow = new VulkanWindow(renderer, _sizeX, _sizeY, _win32Window, _win32Instance);
	_vulkanWindow->initVulcanSurface();
}

void Window::InitWindow() {
	_vulkanWindow->InitVulkanWindow();
	_opened = true;
}

void Window::Close() {
	if(_opened) {
		_vulkanWindow->DeinitVulkanWindow();
		DestroyWindow(_win32Window);
		UnregisterClass(_win32ClassName, _win32Instance);
		_opened = false;
	}
}

void Window::UpdateWindow() {
	MSG msg;
	while(PeekMessage(&msg, _win32Window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(KEYS [KEY_UP] == true) {
		_vulkanWindow->vulkanCamera.cameraPosition-= glm::vec3(0.05f);
		_vulkanWindow->updateCamera();
		KEYS [KEY_UP] = false;
	}

	if(KEYS [KEY_DOWN] == true) {
		_vulkanWindow->vulkanCamera.cameraPosition += glm::vec3(0.05f);
		_vulkanWindow->updateCamera();
		KEYS [KEY_DOWN] = false;
	}
}

bool Window::isOpened() {
	return _opened;
}

const uint32_t Window::GetWidth() const {
	return _sizeX;
}

const uint32_t Window::GetHeight() const {
	return _sizeY;
}

VulkanWindow* Window::GetVulkanWindow() const {
	return _vulkanWindow;
}

void Window::initOSWindow() {
	WNDCLASSEX winClass {};
	winClass.cbSize = sizeof(winClass);
	winClass.style = CS_VREDRAW | CS_HREDRAW;
	winClass.lpfnWndProc = WindowsEventHandler;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = _win32Instance;
	winClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName = NULL;
	winClass.lpszClassName = _win32ClassName;
	winClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if(!RegisterClassEx(&winClass)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT rect = { 0, 0, LONG(_sizeX), LONG(_sizeY) };

	AdjustWindowRectEx(&rect, style, FALSE, ex_style);

	_win32Window = CreateWindowEx(0,
								  winClass.lpszClassName,
								  (LPCSTR) _winTitle, style,
								  CW_USEDEFAULT,
								  CW_USEDEFAULT,
								  rect.right - rect.left,
								  rect.bottom - rect.top,
								  NULL,
								  NULL,
								  _win32Instance,
								  NULL);

	if(!_win32Window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(_win32Window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(_win32Window, SW_SHOW);
	SetForegroundWindow(_win32Window);
	SetFocus(_win32Window);
}

Window::~Window() {
	if(_opened) {
		Close();
	}

	if(_win32Instance || _win32Window) {
		_win32Instance = nullptr;
		_win32Window = nullptr;
	}
}

