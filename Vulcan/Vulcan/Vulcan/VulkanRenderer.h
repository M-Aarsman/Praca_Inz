#pragma once

#include "Shared.h"
#include "Mesh.h"

#include <vector>
#include <stdint.h>
#include <Windows.h>
#include <time.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Window;

struct camera {
	glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
	glm::vec3 lookAt = glm::vec3(0, 0, 0);
};

struct TranslateData {
	TranslateData()
		: X(0.0f),
		Y(0.0f),
		Z(0.0f) {};

	float X;
	float Y;
	float Z;
};

class VulkanRenderer
{
public:
	VulkanRenderer(unsigned int meshNum);
	~VulkanRenderer();
	void OpenWindow(uint32_t sizeX, uint32_t sizeY, const char* name);
	void ExecuteBeginCommandBuffer();
	void ExecuteQueueCommandBuffer();
	bool Run();
	void InitDeviceQueue();
	void InitSemaphore(VkSemaphore* semaphore);
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
	void updateCamera();
	VkViewport* GetViewport();
	const std::vector<VkDescriptorSet> GetDescriptorSet() const;
	const VkSwapchainKHR GetVulcanSwapchainKHR() const;
	const VkSwapchainKHR* pGetVulcanSwapchainKHR() const;
	std::vector<VkImage> GetSwapchainImage() const;
	const VkRenderPass GetRenderPass() const;
	const std::vector<VkFramebuffer> GetFrameBuffer() const;
	const VkPipeline GetPipeline() const;
	const VkPipelineLayout GetPipelineLayout() const;
	void InitRenderPass(uint32_t &imageKHRindex);
	void initVulcanSurface();
	void deinitVulcanSurface();

	camera vulkanCamera;

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
	void initUniformBuffer();
	void deinitUniformBuffer();
	void initDescriptorSet();
	void initDescriptorPool();
	void deinitDescriptorPool();
	void initDescriptorAndPipelineLayout();
	void deinitDescriptorAndPipelineLayout();
	void initPipeline();
	void initPipelineCache();
	void deinitPipelineCache();
	void deinitPipeline();
	void initSwapchain();
	void deinitSwapchain();
	void initSwapchainImages();
	void deinitSwapchainImages();
	void initDepthBuffer();
	void deinitDepthBuffer();
	void setImageLayout();
	void initRenderpass();
	void deinitRenderpass();
	void initShaders();
	void deinitShaders();
	void initframeBuffer();
	void deinitframeBuffer();
	void initVertexBuffer();

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

	VkSemaphore _semaphoreA = VK_NULL_HANDLE;

	uint32_t _imageKHRindex = 0;

	VkImageMemoryBarrier prePresentBarrier {};

	glm::mat4 _projection;
	glm::mat4 _view;
	glm::mat4 _model;
	glm::mat4 _clip;
	glm::mat4 _mvp;

	VkViewport _viewport;
	VkBuffer _uniformDataBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uniformBufferMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo _uniformDescriptorInfo = {};
	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> _descriptorSets;
	std::vector<VkDescriptorSetLayout> _descriptorSetLayout;
	uint32_t _numDescriptorSets = 1;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageView;

	uint32_t _surfaceSizeX = 0;
	uint32_t _surfaceSizeY = 0;
	uint32_t _swapchainImageCount = 2;

	std::vector<VkFramebuffer> _frameBuffers;

	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

	VkRenderPass _renderPass = VK_NULL_HANDLE;

	HINSTANCE _win32Instance = nullptr;
	HWND _win32Window = nullptr;

	VulkanRenderer* _renderer = nullptr;

	VkSurfaceCapabilitiesKHR _surfaceCapabilitesKHR = {};
	VkSurfaceFormatKHR _surfaceFormat = {};

	VkFormat _depthFormat = VK_FORMAT_D16_UNORM;
	VkImage _depthImage = VK_NULL_HANDLE;
	VkImageView _depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory _depthMemory = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo _shaderStagesCreateInfo [2] = {};

	VkVertexInputAttributeDescription _vertexInputAttributeDescription [2] = {};
	VkVertexInputBindingDescription _vertexInputBindingDescription = {};

	VkMemoryRequirements _uniformMemoryRequirements;
	size_t _uniformBufferSize = 0;
	std::vector<TranslateData> _translateValues;
};

