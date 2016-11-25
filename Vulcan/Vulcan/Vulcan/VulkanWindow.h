#pragma once
#include "Shared.h"
#include "Window.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class VulkanRenderer;

struct camera {
	glm::vec3 cameraPosition = glm::vec3(0, 0, 0);
	glm::vec3 lookAt = glm::vec3(0, 0, 0);
};

class VulkanWindow {
	friend class Window;
public:
	VulkanWindow(VulkanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, HWND OSWindow, HINSTANCE OSWindowInstance);
	void InitVulkanWindow();
	void DeinitVulkanWindow();
	const VkSwapchainKHR GetVulcanSwapchainKHR() const;
	const VkSwapchainKHR* pGetVulcanSwapchainKHR() const;
	std::vector<VkImage> GetSwapchainImage() const;
	const VkRenderPass GetRenderPass() const;
	const std::vector<VkFramebuffer> GetFrameBuffer() const;
	const VkPipeline GetPipeline() const;
	const VkPipelineLayout GetPipelineLayout() const;
	const std::vector<VkDescriptorSet> GetDescriptorSet() const;
	void InitRenderPass(uint32_t &imageKHRindex);
	VkViewport* GetViewport();
	void updateCamera();

private:
	void initVulcanSurface();
	void deinitVulcanSurface();
	void initSwapchain();
	void deinitSwapchain();
	void initSwapchainImages();
	void deinitSwapchainImages();
	void initDepthBuffer();
	void deinitDepthBuffer();
	void setImageLayout();
	void initUniformBuffer();
	void deinitUniformBuffer();
	void initDescriptorAndPipelineLayout();
	void deinitDescriptorAndPipelineLayout();
	void initRenderpass();
	void deinitRenderpass();
	void initShaders();
	void deinitShaders();
	void initframeBuffer();
	void deinitframeBuffer();
	void initDescriptorPool();
	void deinitDescriptorPool();
	void initDescriptorSet();
	void initPipelineCache();
	void deinitPipelineCache();
	void initPipeline();
	void deinitPipeline();
	void initVertexBuffer();

public:
	camera vulkanCamera;
private:
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageView;

	uint32_t _surfaceSizeX = 0;
	uint32_t _surfaceSizeY = 0;
	uint32_t _swapchainImageCount = 2;
	uint32_t _numDescriptorSets = 1;

	std::vector<VkFramebuffer> _frameBuffers;

	VkPipeline _pipeline = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
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

	glm::mat4 _projection;
	glm::mat4 _view;
	glm::mat4 _model;
	glm::mat4 _clip;
	glm::mat4 _mvp;

	VkBuffer _uniformDataBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uniformBufferMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo _uniformDescriptorInfo = {};

	bool _useTexture = false;
	bool viewportSet = false;

	std::vector<VkDescriptorSetLayout> _descriptorSetLayout;

	VkPipelineShaderStageCreateInfo _shaderStagesCreateInfo [2] = {};

	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> _descriptorSets;

	VkVertexInputAttributeDescription _vertexInputAttributeDescription [2] = {};
	VkVertexInputBindingDescription _vertexInputBindingDescription = {};

	VkViewport _viewport;
};


