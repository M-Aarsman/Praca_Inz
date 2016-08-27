#pragma once

#include "Shared.h"

#include <vector>
#include <stdint.h>
#include <Windows.h>

class Window;

class VulcanRenderer
{
public:
	VulcanRenderer();
	~VulcanRenderer();
	void OpenWindow(uint32_t sizeX, uint32_t sizeY, const char* name);
	void ExecuteBeginCommandBuffer();
	void ExecuteQueueCommandBuffer();
	bool Run();
	void InitDeviceQueue();
	
	//Getters
	const VkInstance GetVulcanInstance() const;
	const VkPhysicalDevice GetVulcanPhysicalDevice() const;
	const VkDevice GetVulcanDevice() const;
	const VkQueue GetVulcanQueue() const;
	const uint32_t GetVulcanGraphicsQueueFamilyIndex() const;
	const VkPhysicalDeviceProperties& GetVulcanPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties& GetVulcanPhysicalDeviceMemoryProperties() const;
	const VkCommandPool GetVulcanCommandPool() const;
	const VkCommandBuffer GetVulcanCommandBuffer() const;
	Window* GetWindow() const;

private:
	void initVulcanInstance();
	void deinitVulcanInstance();
	void initDevice();
	void deinitDevice();
	void setupDebug();
	void initDebug();
	void deinitDebug();
	void initSurface();
	void deinitSurface();
	void setupLayersAndExtensions();
	void initCommandPool();
	void deinitCommandPool();
	void initCommandBuffer();
	void deinitCommandBuffer();

private:
	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _gpuHandler = VK_NULL_HANDLE;
	VkDevice _deviceHandler = VK_NULL_HANDLE;
	VkQueue _queue = VK_NULL_HANDLE;

	uint32_t _graphicFamilyIndex = 0;
	VkPhysicalDeviceProperties _gpuProperties = {};
	std::vector<const char*> _instanceLayers;
	std::vector<const char*> _instanceExtensions;

	std::vector<const char*> _deviceExtensions;

	VkDebugReportCallbackEXT _debugReportHandler = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo {};
	VkPhysicalDeviceMemoryProperties _memoryProperties;
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;

	Window* _window = nullptr;
	HINSTANCE _windowInstance = nullptr;
	HWND _windowHandler = nullptr;
	uint32_t _winWidth = 0;
	uint32_t _winHeight = 0;
	char* _winTitle = "";
};

