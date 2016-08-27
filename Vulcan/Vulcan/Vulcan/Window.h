#pragma once
#include "Shared.h"
#include <Windows.h>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class VulcanRenderer;

class Window {
public:
	Window(VulcanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, const char* name);
	void InitWindow();
	~Window();
	void Close();
	void UpdateWindow();
	bool isOpened();
	const VkSwapchainKHR GetVulcanSwapchainKHR() const;
	const VkSwapchainKHR* pGetVulcanSwapchainKHR() const;
	std::vector<VkImage> GetSwapchainImage() const;
	const VkRenderPass GetRenderPass() const;
	const std::vector<VkFramebuffer> GetFrameBuffer() const;
	const uint32_t GetWidth() const;
	const uint32_t GetHeight() const;
	const VkPipeline GetPipeline() const;
	const VkPipelineLayout GetPipelineLayout() const;
	const std::vector<VkDescriptorSet> GetDescriptorSet() const;
	const VkBuffer* GetVertexBuffer();

private:
	void initOSWindow();
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
	void initVertexBuffer();
	void deinitVertexBuffer();
	void initDescriptorPool();
	void deinitDescriptorPool();
	void initDescriptorSet();
	void initPipelineCache();
	void deinitPipelineCache();
	void initPipeline();
	void deinitPipeline();

private:
	VulcanRenderer* _renderer = nullptr;
	
	bool _opened = false;
	bool _useTexture = false;
	
	HINSTANCE _win32Instance = nullptr;
	HWND _win32Window = nullptr;
	LPSTR _win32ClassName;
	
	uint32_t _sizeX = 0;
	uint32_t _sizeY = 0;
	uint32_t _surfaceSizeX = 0;
	uint32_t _surfaceSizeY = 0;
	uint32_t _swapchainImageCount = 2;
	uint32_t _numDescriptorSets = 1;

	const char* _winTitle;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageView;
	std::vector<VkDescriptorSetLayout> _descriptorSetLayout;
	std::vector<VkFramebuffer> _frameBuffers;
	std::vector<VkDescriptorSet> _descriptorSets;

	VkSurfaceCapabilitiesKHR _surfaceCapabilitesKHR = {};
	VkSurfaceFormatKHR _surfaceFormat = {};
	VkPipelineShaderStageCreateInfo _shaderStagesCreateInfo [2] = {};
	VkVertexInputAttributeDescription _vertexInputAttributeDescription [2] = {};
	VkVertexInputBindingDescription _vertexInputBindingDescription = {};
	VkDescriptorBufferInfo _uniformDescriptorInfo = {};

	VkImage _depthImage = VK_NULL_HANDLE;
	VkImageView _depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory _depthMemory = VK_NULL_HANDLE;
	VkBuffer _uniformDataBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uniformBufferMemory = VK_NULL_HANDLE;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkFormat _depthFormat = VK_FORMAT_D16_UNORM;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	VkBuffer _vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
	VkPipelineCache _pipelineCache = VK_NULL_HANDLE;
	VkPipeline _pipeline = VK_NULL_HANDLE;

	glm::mat4 _projection;
	glm::mat4 _view;
	glm::mat4 _model;
	glm::mat4 _clip;
	glm::mat4 _mvp;
};

