#pragma once

#include <vulkan\vulkan.h>

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

private:
	VkInstance _instance = nullptr;
	VkPhysicalDevice _gpuHandler = nullptr;
	VkDevice _deviceHandler = nullptr;
	uint32_t _graphicFamilyIndex = 0;
	VkPhysicalDeviceProperties _gpuProperties = {};
};

