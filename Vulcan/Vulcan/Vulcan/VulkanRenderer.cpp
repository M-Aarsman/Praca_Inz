#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanWindow.h"

//#include "Mesh.h"
#include <cstdlib>
#include <iostream>
#include <stdio.h>


VulkanRenderer::VulkanRenderer(unsigned int meshNum) 
	: _meshNum(meshNum) {
	setupLayersAndExtensions();
	setupDebug();
	initVulcanInstance();
	initDebug();
	initDevice();
	initCommandPool();
	initCommandBuffer();
	

	//TODO: should take it in some other place than constructor

	loadVertx("cubeVertex.txt");
	meshes.push_back(new Mesh(&vertices, 0, vertices.size()));
	int begin = vertices.size();
	_vertexPerMesh = vertices.size();
	int end = begin + _vertexPerMesh;

	// I want to put mesh togeter in some kind of square

	unsigned int meshPerRow = (unsigned int) (sqrt(_meshNum));

	unsigned int centerIndex = meshPerRow / 2; // the cube index (i,j) which will be around 0,0

	//now we have too have smoewhere all of vertices for all cube! (TODO: optimalize someday)
	//hope this not take ages to copy it....

	for(size_t i = 0; i < _vertexPerMesh * (_meshNum - 1); i++) {
		vertices.push_back(vertices [i % _vertexPerMesh]);
	}

	//TODO: !!! calculate object width + object height

	int objectWidth = 4;
	int objectHeight = 4;

	//create Meshes
	for(size_t i = 1; i < meshNum; i++) {
		unsigned int begin = _vertexPerMesh * i;
		end = _vertexPerMesh * i + _vertexPerMesh;
		meshes.push_back(new Mesh(&vertices,begin, end));
	}

	// and now translate it...

	for(int i = 0; i < meshPerRow; i++) {  // +/- flip
		for(int j = 0; j < meshPerRow; j++) {

			int currentIndex = i*meshPerRow + j;
			if((i*meshPerRow + j) >= meshNum) {
				break;
			}

			float traslateValueY = (float)(((int)centerIndex - i) * objectWidth);
			float traslateValueX = (float) (((int)centerIndex - j) * objectHeight);

			meshes [i*meshPerRow + j]->TranslateX(traslateValueX);
			meshes [i*meshPerRow + j]->TranslateY(traslateValueY);
		}
	}

	//meshes.push_back(new Mesh(&vertices, begin, end));
	//meshes [1]->TranslateX(-6);
}

VulkanRenderer::~VulkanRenderer() {
	deinitCommandBuffer();
	deinitCommandPool();
	deinitDevice();
	deinitDebug();
	deinitVulcanInstance();
	if(_window != nullptr) {
		delete(_window);
		_window = nullptr;
	}
}

void VulkanRenderer::initVulcanInstance() {
	VkApplicationInfo applicationInfo {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
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

void VulkanRenderer::setupLayersAndExtensions() {
	_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	_instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

}

void VulkanRenderer::initCommandPool() {
	VkCommandPoolCreateInfo commandPoolCreateInfo {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = NULL;
	commandPoolCreateInfo.queueFamilyIndex = _graphicFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	ErrorCheck(vkCreateCommandPool(_deviceHandler, &commandPoolCreateInfo, nullptr, &_commandPool));
}

void VulkanRenderer::deinitCommandPool() {
	vkDestroyCommandPool(_deviceHandler, _commandPool, nullptr);
}

void VulkanRenderer::initCommandBuffer() {
	VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = _commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	ErrorCheck(vkAllocateCommandBuffers(_deviceHandler, &commandBufferAllocateInfo, &_commandBuffer));
}

void VulkanRenderer::deinitCommandBuffer() {
	vkFreeCommandBuffers(_deviceHandler, _commandPool, 1, &_commandBuffer);
}

void VulkanRenderer::initDevice() {
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

	vkGetPhysicalDeviceMemoryProperties(_gpuHandler, &_memoryProperties);
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

}

void VulkanRenderer::deinitDevice() {
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

void VulkanRenderer::setupDebug() {
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

void VulkanRenderer::initDebug() {
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

	if(fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr) {
		assert(0 && "VULCAN ERR: Can not fetch debug function pointers!"); 
		std::exit(-1);
	}

	fvkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugReportHandler);
}

void VulkanRenderer::deinitDebug() {
	fvkDestroyDebugReportCallbackEXT(_instance, _debugReportHandler, nullptr);
	_debugReportHandler = VK_NULL_HANDLE;
}

void VulkanRenderer::initSurface() {}

void VulkanRenderer::deinitSurface() {}

void VulkanRenderer::OpenWindow(uint32_t sizeX, uint32_t sizeY, const char * name) {
	_window = new Window(this, sizeX, sizeY, name);
}

void VulkanRenderer::ExecuteBeginCommandBuffer() {
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	ErrorCheck(vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo));
}

void VulkanRenderer::ExecuteQueueCommandBuffer() {
	const VkCommandBuffer cmd_bufs [] = { _commandBuffer};
	VkFenceCreateInfo fenceInfo  {};
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(_deviceHandler, &fenceInfo, nullptr, &drawFence);

	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submitInfo [1] = {};
	submitInfo [0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo [0].pWaitDstStageMask = &pipe_stage_flags;
	submitInfo [0].commandBufferCount = 1;
	submitInfo [0].pCommandBuffers = cmd_bufs;

	ErrorCheck(vkQueueSubmit(_queue, 1, submitInfo, drawFence));

	VkResult result = VK_SUCCESS;
	do {
		result = vkWaitForFences(_deviceHandler, 1, &drawFence, VK_TRUE, UINT64_MAX);
	} while(result == VK_TIMEOUT);

	vkDestroyFence(_deviceHandler, drawFence, nullptr);
}

bool VulkanRenderer::Run(uint32_t &imageKHRindex) {

	VkSemaphore semaphoreA = VK_NULL_HANDLE;
	InitSemaphore(&semaphoreA, imageKHRindex);
	if(begin == 0)
		begin = clock();

	if(_window != nullptr) {
		_window->UpdateWindow();

		auto res = VK_SUCCESS;

		clock_t end = clock();

		if(double(end - begin) / CLOCKS_PER_SEC > 0.05) {
			for(int i = 0; i < _meshNum; i++) {
				meshes [i]->RotateZ(3);
			}

			begin = clock();
		}

		uint8_t *pData;
		ErrorCheck(vkMapMemory(_deviceHandler, _vertexBufferMemory, 0, _memoryReqs.size, 0, (void **) &pData));

		memcpy(pData, vertices.data(), _meshNum * _vertexPerMesh* sizeof(Vertex));
		vkUnmapMemory(_deviceHandler, _vertexBufferMemory);

		_window->GetVulkanWindow()->InitRenderPass(imageKHRindex);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _window->GetVulkanWindow()->GetPipeline());
		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _window->GetVulkanWindow()->GetPipelineLayout(),
								0, 1, _window->GetVulkanWindow()->GetDescriptorSet().data(), 0, nullptr);

		const VkDeviceSize offsets [1] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_vertexBuffer, offsets);
		Draw(imageKHRindex, &semaphoreA);
		vkDestroySemaphore(_deviceHandler, semaphoreA, nullptr);
		ExecuteBeginCommandBuffer();

		return _window->isOpened();
	}
	vkDestroySemaphore(_deviceHandler, semaphoreA, nullptr);
	ExecuteBeginCommandBuffer();
	return true;
}

void VulkanRenderer::InitDeviceQueue() {
	vkGetDeviceQueue(_deviceHandler, _graphicFamilyIndex, 0, &_queue);
}

void VulkanRenderer::set_image_layout(VkImage& image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
					  VkImageLayout new_image_layout) {

	assert(_commandBuffer != VK_NULL_HANDLE);
	assert(_queue != VK_NULL_HANDLE);

	VkImageMemoryBarrier imageMemoryBarier {};
	imageMemoryBarier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;;
	imageMemoryBarier.oldLayout = old_image_layout;
	imageMemoryBarier.newLayout = new_image_layout;
	imageMemoryBarier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarier.image = image;
	imageMemoryBarier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarier.subresourceRange.levelCount = 1;
	imageMemoryBarier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarier.subresourceRange.layerCount = 1;

	if(old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		imageMemoryBarier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	}

	if(new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if(new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(_commandBuffer, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarier);
}

void VulkanRenderer::loadVertx(char * fileName) {
	FILE* fp;

	if(!(fp = fopen(fileName, "r"))) {
		assert(0 && "Can not open file!");
	}

	int size = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	if(!size) {
		assert(0 && "file is empty!");
	}

	fseek(fp, 0, SEEK_SET);

	char c;
	bool position = true;
	bool comment = false;

	float value [4] = { 0 };
	char buffer [32] = { 0 };

	Vertex current;

	uint8_t valueConuter = 0;
	uint8_t charCounter = 0;

	//read file

	while((c = getc(fp)) != EOF) {
		if(c == '#') {
			comment = true;
			continue;
		}

		if(c == '\n') {
			if(comment) {
				comment = false;
				continue;
			}

			if(!position) {
				position = true;

				//flush value
				if(buffer [0] == '\0') {// more than 1 space
					continue;
				}

				if(valueConuter > 4) {
					assert(0 && "Invalid data in file");
				}

				value [valueConuter] = atof(buffer);
				valueConuter++;

				//clear buffer
				for(int i = 0; i < 32; i++) {
					buffer [i] = '\0';
				}

				charCounter = 0;

				if(valueConuter != 4) { // too much or less than shuld be 
					assert(0 && "Invalid data in file");
				}
				valueConuter = 0;
				//flush color
				current.color = glm::vec4(value [0], value [1], value [2], value [3]);

				//flush vertex
				vertices.push_back(current);

				continue;
			}

			assert(0 && "Invalid data in file");
		}

		if(comment) {
			continue;
		}

		if(c == ';') {
			position = false;

			//flush value
			if(buffer [0] == '\0') {// more than 1 space
				continue;
			}

			if(valueConuter > 4) {
				assert(0 && "Invalid data in file");
			}

			value [valueConuter] = atof(buffer);
			valueConuter++;

			//clear buffer
			for(int i = 0; i < 32; i++) {
				buffer [i] = '\0';
			}

			charCounter = 0;

			if(valueConuter != 4) { // too much or less than shuld be 
				assert(0 && "Invalid data in file");
			}

			valueConuter = 0;
			//flush postion
			current.position = glm::vec4(value [0], value [1], value [2], value [3]);

			continue;
		}

		if(c == ' ' || c == '\t') {
			continue;
		}

		if(c == ',') {
			//flush value
			if(buffer [0] == '\0') {// smotething wrong meybe 2 separators?
				assert(0 && "Invalid data in file");
			}

			if(valueConuter > 4) {
				assert(0 && "Invalid data in file");
			}

			value [valueConuter] = atof(buffer);
			valueConuter++;

			//clear buffer
			for(int i = 0; i < 32; i++) {
				buffer [i] = '\0';
			}

			charCounter = 0;
			continue;
		}

		buffer [charCounter] = c;
		charCounter++;
	}
}

void VulkanRenderer::InitSemaphore(VkSemaphore* semaphore, uint32_t &imageKHRindex) {
	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo {};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ErrorCheck(vkCreateSemaphore(_deviceHandler, &presentCompleteSemaphoreCreateInfo, nullptr, semaphore));

	ErrorCheck(vkAcquireNextImageKHR(_deviceHandler, _window->GetVulkanWindow()->GetVulcanSwapchainKHR(), UINT64_MAX, *semaphore, VK_NULL_HANDLE, &imageKHRindex));

	set_image_layout(_window->GetVulkanWindow()->GetSwapchainImage() [imageKHRindex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
					 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

const VkInstance VulkanRenderer::GetVulcanInstance() const {
	return _instance;
}

const VkPhysicalDevice VulkanRenderer::GetVulcanPhysicalDevice() const {
	return _gpuHandler;
}

const VkDevice VulkanRenderer::GetVulcanDevice() const {
	return _deviceHandler;
}

const VkQueue VulkanRenderer::GetVulcanQueue() const {
	return _queue;
}

const uint32_t VulkanRenderer::GetVulcanGraphicsQueueFamilyIndex() const {
	return _graphicFamilyIndex;
}

const VkPhysicalDeviceProperties & VulkanRenderer::GetVulcanPhysicalDeviceProperties() const {
	return _gpuProperties;
}

const VkPhysicalDeviceMemoryProperties & VulkanRenderer::GetVulcanPhysicalDeviceMemoryProperties() const {
	return _memoryProperties;
}

const VkCommandPool VulkanRenderer::GetVulcanCommandPool() const {
	return _commandPool;
}

const VkCommandBuffer VulkanRenderer::GetVulcanCommandBuffer() const {
	return _commandBuffer;
}

Window * VulkanRenderer::GetWindow() const {
	return _window;
}

const VkBuffer* VulkanRenderer::GetVertexBuffer() {
	return &_vertexBuffer;
}

void VulkanRenderer::deinitVulcanInstance() {
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void VulkanRenderer::InitVertexBuffer() {

	VkBufferCreateInfo buffferCreateInfo {};
	buffferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffferCreateInfo.size =  _meshNum * _vertexPerMesh * sizeof(Vertex);
	buffferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ErrorCheck(vkCreateBuffer(_deviceHandler, &buffferCreateInfo, nullptr, &_vertexBuffer));

	
	vkGetBufferMemoryRequirements(_deviceHandler, _vertexBuffer, &_memoryReqs);

	VkMemoryAllocateInfo allocateInfo {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = _memoryReqs.size;

	uint32_t memoryTypeCount = _memoryProperties.memoryTypeCount;
	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((_memoryReqs.memoryTypeBits & 1) == 1) {
			if((_memoryProperties.memoryTypes [i].propertyFlags &
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				allocateInfo.memoryTypeIndex = i;
			}
		}
		_memoryReqs.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(_deviceHandler, &allocateInfo, nullptr, &_vertexBufferMemory));

	ErrorCheck(vkBindBufferMemory(_deviceHandler, _vertexBuffer, _vertexBufferMemory, 0));
}

void VulkanRenderer::DeinitVertexBuffer() {
	vkDestroyBuffer(_deviceHandler, _vertexBuffer, nullptr);
	vkFreeMemory(_deviceHandler, _vertexBufferMemory, nullptr);
}

void VulkanRenderer::Draw(uint32_t &imageKHRindex, VkSemaphore* semaphore) {
	vkCmdDraw(_commandBuffer, _meshNum * _vertexPerMesh, 1, 0, 0);
	vkCmdEndRenderPass(_commandBuffer);

	VkImageMemoryBarrier prePresentBarrier {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.layerCount = 1;
	prePresentBarrier.image = _window->GetVulkanWindow()->GetSwapchainImage() [imageKHRindex];

	vkCmdPipelineBarrier(_commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

	const VkCommandBuffer cmdBufs [] = { _commandBuffer };
	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;

	ErrorCheck(vkCreateFence(_deviceHandler, &fenceInfo, nullptr, &_drawFence));

	VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submitInfo [1] = {};
	submitInfo [0].pNext = NULL;
	submitInfo [0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo [0].waitSemaphoreCount = 1;
	submitInfo [0].pWaitSemaphores = semaphore;
	submitInfo [0].pWaitDstStageMask = &pipelineStageFlags;
	submitInfo [0].commandBufferCount = 1;
	submitInfo [0].pCommandBuffers = cmdBufs;
	submitInfo [0].signalSemaphoreCount = 0;
	submitInfo [0].pSignalSemaphores = NULL;

	ErrorCheck(vkEndCommandBuffer(_commandBuffer));
	ErrorCheck(vkQueueSubmit(_queue, 1, submitInfo, _drawFence));

	VkPresentInfoKHR present {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.swapchainCount = 1;
	present.pSwapchains = _window->GetVulkanWindow()->pGetVulcanSwapchainKHR();
	present.pImageIndices = &imageKHRindex;

	auto res = VK_SUCCESS;
	do {
		res = vkWaitForFences(_deviceHandler, 1, &_drawFence, VK_TRUE, UINT64_MAX);
	} while(res == VK_TIMEOUT);

	assert(res == VK_SUCCESS);
	ErrorCheck(vkQueuePresentKHR(_queue, &present));
	vkDestroyFence(_deviceHandler, _drawFence, nullptr);
}
