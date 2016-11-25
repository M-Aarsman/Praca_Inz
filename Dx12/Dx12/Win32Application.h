#pragma once
#include <Windows.h>
#include "Dx12Renderer.h"
#include <string>

class Win32Application {
public:
	Win32Application(int width, int height, std::wstring name);
	int Run(Dx12Renderer* renderer, HINSTANCE hInstance, int nCmdShow);
	
	HWND GetHwnd() { return _hwnd; }
	int GetWidth() {
		return _width;
	}
	int GetHeight() {
		return _height;
	}

	LPCWSTR GetTitle() {
		return (LPCWSTR) _title.c_str();
	}

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND _hwnd;

	//Window Parameters
	int _width;
	int _height;
	std::wstring _title;
};

