#pragma once

#include <vulkan\vulkan.h>
#include <vector>

class VulcanRenderer
{
public:
	VulcanRenderer();
	~VulcanRenderer();

private:
	void initVulcanInstance();
	void deinitVulcanInstance();
	void initDevice();
	void deinitDevice();
	void setupDebug();
	void initDebug();
	void deinitDebug();

private:
	VkInstance _instance = nullptr;
	VkPhysicalDevice _gpuHandler = nullptr;
	VkDevice _deviceHandler = nullptr;
	uint32_t _graphicFamilyIndex = 0;
	VkPhysicalDeviceProperties _gpuProperties = {};
	std::vector<const char*> _instanceLayers;
	std::vector<const char*> _instanceExtensions;

	VkDebugReportCallbackEXT _debugReportHandler = nullptr;
	VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo {};
};

