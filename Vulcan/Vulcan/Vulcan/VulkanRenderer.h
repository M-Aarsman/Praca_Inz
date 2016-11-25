#pragma once

#include "Shared.h"
#include "Mesh.h"

#include <vector>
#include <stdint.h>
#include <Windows.h>
#include <time.h>

class Window;

class VulkanRenderer
{
public:
	VulkanRenderer(unsigned int meshNum);
	~VulkanRenderer();
	void OpenWindow(uint32_t sizeX, uint32_t sizeY, const char* name);
	void ExecuteBeginCommandBuffer();
	void ExecuteQueueCommandBuffer();
	bool Run(uint32_t &imageKHRindex);
	void InitDeviceQueue();
	void InitSemaphore(VkSemaphore* semaphore, uint32_t &imageKHRindex);
	void InitVertexBuffer();
	void DeinitVertexBuffer();
	
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
	const VkBuffer* GetVertexBuffer();
	void Draw(uint32_t &imageKHRindex, VkSemaphore* semaphore);

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

	void set_image_layout(VkImage& image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
						  VkImageLayout new_image_layout);

	void loadVertx(char* fileName);

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

	clock_t begin = 0;

	VkBuffer _vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
	VkFence _drawFence = VK_NULL_HANDLE;

	VkMemoryRequirements _memoryReqs;

	std::vector<Vertex> vertices;
	
	std::vector<Mesh*> meshes;

	unsigned int _vertexPerMesh = 0;
	unsigned int _meshNum;
};

