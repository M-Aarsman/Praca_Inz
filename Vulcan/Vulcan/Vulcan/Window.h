#pragma once
#include "Shared.h"
#include <Windows.h>

class VulcanRenderer;

class Window {
public:
	Window(VulcanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, const char* name);
	~Window();
	void Close();
	void UpdateWindow();
	bool isOpened();

private:
	void initOSWindow();
	void initVulcanSurface();
	void deinitVulcanSurface();
	void initSwapchain();
	void deinitSwapchain();

private:
	VulcanRenderer* _renderer = nullptr;
	
	bool _opened = false;
	
	HINSTANCE _win32Instance = nullptr;
	HWND _win32Window = nullptr;
	LPSTR _win32ClassName;
	
	uint32_t _sizeX = 0;
	uint32_t _sizeY = 0;
	uint32_t _surfaceSizeX = 0;
	uint32_t _surfaceSizeY = 0;
	uint32_t _swapchainImageCount = 2;

	const char* _winTitle;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkSurfaceCapabilitiesKHR _surfaceCapabilitesKHR = {};
	VkSurfaceFormatKHR _surfaceFormat = {};

};

