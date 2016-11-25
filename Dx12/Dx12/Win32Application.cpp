#include "Win32Application.h"

HWND Win32Application::_hwnd = nullptr;

Win32Application::Win32Application(int width, int height, std::wstring name)
	: _width(width),
	 _height(height),
	 _title(name)
{}

int Win32Application::Run(Dx12Renderer * renderer, HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	_hwnd = CreateWindow(
		windowClass.lpszClassName,
		_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		renderer);

	renderer->SetWindowHeight(_height);
	renderer->SetWindowWidth(_width);
	renderer->OnInit();

	ShowWindow(_hwnd, nCmdShow);

	// Main sample loop.
	MSG msg = {};
	while(msg.message != WM_QUIT) {
		// Process any messages in the queue.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	renderer->OnDestroy();

	return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	Dx12Renderer* renderer = reinterpret_cast<Dx12Renderer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch(message) {
		case WM_CREATE:
		{
			// Save the DXSample* passed in to CreateWindow.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

		case WM_KEYDOWN:
		if(renderer) {
			renderer->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

		case WM_KEYUP:
		if(renderer) {
			renderer->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

		case WM_PAINT:
		if(renderer) {
			renderer->OnUpdate();
			renderer->OnRender();
		}
		return 0;

		case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);

}
