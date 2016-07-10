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

private:
	VkInstance instance = nullptr;
};

