#include "VulcanRenderer.h"
#include "Shared.h"
#include "Window.h"

#include <cstdlib>
#include <iostream>

VulcanRenderer::VulcanRenderer() {
	setupLayersAndExtensions();
	setupDebug();
	initVulcanInstance();
	initDebug();
	initDevice();
}

VulcanRenderer::~VulcanRenderer() {
	deinitDevice();
	deinitDebug();
	deinitVulcanInstance();
	if(_window != nullptr) {
		delete(_window);
		_window = nullptr;
	}
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

void VulcanRenderer::setupLayersAndExtensions() {
	_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	_instanceExtensions.push_back("VK_KHR_win32_surface");
	//_instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

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
	deviceCreateInfo.enabledExtensionCount = _deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensions.data();

	ErrorCheck(vkCreateDevice(_gpuHandler, &deviceCreateInfo, nullptr, &_deviceHandler));

	vkGetDeviceQueue(_deviceHandler, _graphicFamilyIndex, 0, &_queue);

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
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		assert(0 && "VULCAN ERROR");
	}
	return false;
}

void VulcanRenderer::setupDebug() {
	debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugCallbackCreateInfo.pfnCallback = DebugCallback;
	debugCallbackCreateInfo.flags = //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
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
	_debugReportHandler = VK_NULL_HANDLE;
}

void VulcanRenderer::initSurface() {}

void VulcanRenderer::deinitSurface() {}

void VulcanRenderer::OpenWindow(uint32_t sizeX, uint32_t sizeY, const char * name) {
	_window = new Window(this, sizeX, sizeY, name);
}

bool VulcanRenderer::Run() {
	if(_window != nullptr) {
		_window->UpdateWindow();
		return _window->isOpened();
	}
	
	return true;
}

const VkInstance VulcanRenderer::GetVulcanInstance() const {
	return _instance;
}

const VkPhysicalDevice VulcanRenderer::GetVulcanPhysicalDevice() const {
	return _gpuHandler;
}

const VkDevice VulcanRenderer::GetVulcanDevice() const {
	return _deviceHandler;
}

const VkQueue VulcanRenderer::GetVulcanQueue() const {
	return _queue;
}

const uint32_t VulcanRenderer::GetVulcanGraphicsQueueFamilyIndex() const {
	return _graphicFamilyIndex;
}

const VkPhysicalDeviceProperties & VulcanRenderer::GetVulcanPhysicalDeviceProperties() const {
	return _gpuProperties;
}

void VulcanRenderer::deinitVulcanInstance() {
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}
