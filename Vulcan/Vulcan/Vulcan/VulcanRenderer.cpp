#include "VulcanRenderer.h"
#include <cstdlib>
#include <assert.h>
#include <vector>

VulcanRenderer::VulcanRenderer() {
	initVulcanInstance();
	initDevice();
}


VulcanRenderer::~VulcanRenderer() {
	deinitDevice();
	deinitVulcanInstance();
}

void VulcanRenderer::initVulcanInstance() {
	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	VkResult error = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);

	if (error != VK_SUCCESS) {
		assert(0 && "VULCAN ERR: Vulcan create instance error!"); //TODO: add log file and log macro
		std::exit(-1);
	}
}

void VulcanRenderer::deinitVulcanInstance() {
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void VulcanRenderer::initDevice() {
	{
		uint32_t gpuCount = 0;
		//once for get gpuCount
		vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
		std::vector<VkPhysicalDevice> gpuDevices(gpuCount);
		//second to get list of device
		vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuDevices.data());
		_gpuHandler = gpuDevices [0];
		vkGetPhysicalDeviceProperties(_gpuHandler, &_gpuProperties);
	}

	//ask gpu about famili queue it has
	{
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuHandler, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> familyPropeties(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuHandler, &familyCount, familyPropeties.data());

		//check if I had family queue that support graphic
		bool found = false;
		for(uint32_t i = 0; i < familyCount; i++) {
			if(familyPropeties [i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				found = true;
				_graphicFamilyIndex = i;
			}
		}

		if(!found) {
			assert(0 && "VULCAN ERR: Queue Family supported graphics not found!");
			std::exit(-2);
		}
	}

	//one value b. I use 1 family for now
	float queuePriorite [] { 1.0f };

	VkDeviceQueueCreateInfo deviceQueueCreateInfo {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = _graphicFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorite;

	VkDeviceCreateInfo deviceCreateInfo {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

	auto error = vkCreateDevice(_gpuHandler, &deviceCreateInfo, nullptr, &_deviceHandler);
	if(error != VK_SUCCESS) {
		assert(0 && "VULCAN ERR: Create device error!");
		std::exit(-3);
	}
}

void VulcanRenderer::deinitDevice() {
	vkDestroyDevice(_deviceHandler, nullptr);
	_deviceHandler = nullptr;
}
