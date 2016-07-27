#include "VulcanRenderer.h"
#include "Shared.h"

#include <cstdlib>
#include <assert.h>
#include <iostream>

VulcanRenderer::VulcanRenderer() {
	setupDebug();
	initVulcanInstance();
	initDebug();
	initDevice();
}


VulcanRenderer::~VulcanRenderer() {
	deinitDevice();
	deinitDebug();
	deinitVulcanInstance();
}

void VulcanRenderer::initVulcanInstance() {
	VkApplicationInfo applicationInfo {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	applicationInfo.pApplicationName = "App 1";


	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = _instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayers.data();
	instanceCreateInfo.enabledExtensionCount = _instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensions.data();
	instanceCreateInfo.pNext = &debugCallbackCreateInfo;

	ErrorCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
}

void VulcanRenderer::initDevice() {
	{
		uint32_t gpuCount = 0;
		//once for get gpuCount
		ErrorCheck(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));

		std::vector<VkPhysicalDevice> gpuDevices(gpuCount);
		//second to get list of device
		ErrorCheck(vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuDevices.data()));
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
	//enumerate dubug instance layers
	{
		uint32_t layerCount = 0;
		ErrorCheck(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		std::vector<VkLayerProperties> layerProperties(layerCount);
		ErrorCheck(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));

		std::cout << "Instance layers properties:\n";

		for(auto &i : layerProperties) {
			std::cout << "  " << i.layerName << "\t\t |" << i.description << std::endl;
		}

		std::cout << std::endl;

	}

	//enumerate dubug device layers
	{
		uint32_t layerCount = 0;
		ErrorCheck(vkEnumerateDeviceLayerProperties(_gpuHandler, &layerCount, nullptr));
		std::vector<VkLayerProperties> layerProperties(layerCount);
		ErrorCheck(vkEnumerateDeviceLayerProperties(_gpuHandler, &layerCount, layerProperties.data()));

		std::cout << "Device layers properties:\n";

		for(auto &i : layerProperties) {
			std::cout << "  " << i.layerName << "\t\t |" << i.description << std::endl;
		}

		std::cout << std::endl;

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

	ErrorCheck(vkCreateDevice(_gpuHandler, &deviceCreateInfo, nullptr, &_deviceHandler));

}

void VulcanRenderer::deinitDevice() {
	vkDestroyDevice(_deviceHandler, nullptr);
	_deviceHandler = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
			  uint64_t sourceObject, size_t location, int32_t msgCode,
			  const char* layerPrefix, const char* msg, void* userData) 
{
	if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		std::cout << "INFO:  ";
	} 
	if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		std::cout << "WARNING:  ";
	}
	if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		std::cout << "PERFORMANCE_WARNING:  ";
	}
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		std::cout << "ERROR:  ";
	}
	if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		std::cout << "DEBUG:  ";
	}
	std::cout << "[" << layerPrefix << "]: ";
	std::cout << msg << std::endl;
	return false;
}

void VulcanRenderer::setupDebug() {
	debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugCallbackCreateInfo.pfnCallback = DebugCallback;
	debugCallbackCreateInfo.flags =// VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
									VK_DEBUG_REPORT_WARNING_BIT_EXT |
									VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
									VK_DEBUG_REPORT_ERROR_BIT_EXT |
									//VK_DEBUG_REPORT_DEBUG_BIT_EXT | 
									0;

	_instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");

	_instanceExtensions.push_back("VK_EXT_debug_report");
}

PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;

void VulcanRenderer::initDebug() {
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

	if(fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr) {
		assert(0 && "VULCAN ERR: Can not fetch debug function pointers!"); 
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugReportHandler);
}

void VulcanRenderer::deinitDebug() {
	fvkDestroyDebugReportCallbackEXT(_instance, _debugReportHandler, nullptr);
	_debugReportHandler = nullptr;
}

void VulcanRenderer::deinitVulcanInstance() {
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}
