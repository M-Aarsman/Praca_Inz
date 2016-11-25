#pragma once
#include "Shared.h"
#include <Windows.h>
#include <vector>

class VulkanWindow;
class VulkanRenderer;

class Window {
public:
	Window(VulkanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, const char* name);
	void InitWindow();
	~Window();
	void Close();
	void UpdateWindow();
	bool isOpened();
	const uint32_t GetWidth() const;
	const uint32_t GetHeight() const;
	VulkanWindow* GetVulkanWindow() const;

private:
	void initOSWindow();

private:	
	bool _opened = false;
	
	HINSTANCE _win32Instance = nullptr;
	HWND _win32Window = nullptr;
	LPSTR _win32ClassName;
	
	uint32_t _sizeX = 0;
	uint32_t _sizeY = 0;

	const char* _winTitle;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	
	VulkanWindow* _vulkanWindow = nullptr;
	VulkanRenderer* _renderer = nullptr;
};

