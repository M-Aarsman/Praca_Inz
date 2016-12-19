#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanRenderer.h"
#include "Shaders.h"

//#include "Mesh.h"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <array>


VulkanRenderer::VulkanRenderer(unsigned int meshNum)
	: _meshNum(meshNum) {

	setupLayersAndExtensions();
	setupDebug();
	initVulcanInstance();
	initDebug();
	initDevice();

	OpenWindow(600, 600, "First Window");
	_winWidth = 600;
	_winHeight = 600;
	initCommandPool();
	initCommandBuffer();
	initSwapchain();
	initSwapchainImages();
	ExecuteBeginCommandBuffer();
	InitDeviceQueue();
	initDepthBuffer();
	initUniformBuffer();
	initDescriptorAndPipelineLayout();
	initRenderpass();
	initShaders();
	initframeBuffer();
	initVertexBuffer();
	initDescriptorPool();
	initDescriptorSet();
	initPipelineCache();
	initPipeline();

	//TODO: should take it in some other place than constructor

	loadVertx("cubeVertex.txt");
	meshes.push_back(new Mesh(&vertices, 0, vertices.size()));
	int begin = vertices.size();
	_vertexPerMesh = vertices.size();

	_viewport.height = (float) _winHeight;
	_viewport.width = (float) _winWidth;
	_viewport.minDepth = (float)0.0f;
	_viewport.maxDepth = (float)1.0f;
	_viewport.x = 0;
	_viewport.y = 0;

	vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);

	VkRect2D rect2d;
	rect2d.extent.width = _winWidth;
	rect2d.extent.height = _winHeight;
	rect2d.offset.x = 0;
	rect2d.offset.y = 0;
	vkCmdSetScissor(_commandBuffer, 0, 1, &rect2d);

	InitVertexBuffer();
	_window->InitWindow();

	float fov = glm::radians(45.0f);
	if(_winWidth > _winHeight) {
		fov *= static_cast<float>(_winHeight) / static_cast<float>(_winWidth);
	}

	_projection = glm::perspective(fov, static_cast<float>(_winWidth) / static_cast<float>(_winHeight), 0.1f, 100.0f);

	vulkanCamera.cameraPosition = glm::vec3(5, 3, 10);

	_view = glm::lookAt(vulkanCamera.cameraPosition,//Camera Position,
						glm::vec3(0, 0, 0), // look at
						glm::vec3(0, -1, 0) /*head up */);
	_model = glm::mat4(1.0f);

	// Vulkan clip space has inverted Y and half Z.
	_clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, -1.0f, 0.0f, 0.0f,
					  0.0f, 0.0f, 0.5f, 0.0f,
					  0.0f, 0.0f, 0.5f, 1.0f);

	_vertexPerMesh = vertices.size();

	// I want to put mesh togeter in some kind of square

	unsigned int rowNum = (unsigned int) (sqrt(_meshNum));
	unsigned int meshPerRow = _meshNum / rowNum;

	unsigned int centerIndex = meshPerRow / 2; // the cube index (i,j) which will be around 0,0

											   //TODO: !!! calculate object width + object heigh
	int objectWidth = 3;
	int objectHeight = 3;

	//create Meshes

	for(int i = 0; i <= rowNum; i++) {  // +/- flip
		for(int j = 0; j < meshPerRow; j++) {

			int currentIndex = i*meshPerRow + j;
			if(currentIndex >= _meshNum) {
				break;
			}

			float traslateValueY = (float) (((int) centerIndex - i) * objectWidth);
			float traslateValueX = (float) (((int) centerIndex - j) * objectHeight);

			TranslateData trans;
			trans.X = traslateValueX;
			trans.Y = traslateValueY;

			_translateValues.push_back(trans);
		}
	}

	uint8_t *pData = (uint8_t*) malloc(_meshNum * _uniformBufferSize);
	ErrorCheck(vkMapMemory(_deviceHandler, _uniformBufferMemory, 0, _uniformMemoryRequirements.size, 0, (void **) &pData));
	for(int i = 0; i < _meshNum; i++) {
		glm::mat4x4 model = glm::translate(_model, glm::vec3(_translateValues [i].X, _translateValues [i].Y, _translateValues [i].Z));
		_mvp = _clip * _projection * _view * model;
		memcpy(pData + i * _uniformBufferSize, &_mvp, sizeof(_mvp));
	}
	vkUnmapMemory(_deviceHandler, _uniformBufferMemory);
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
	_win32Window = _window->GetHWND();
	_win32Instance = _window->GetWindowHINSTANCE();
	initVulcanSurface();
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

bool VulkanRenderer::Run() {
	InitSemaphore(&_semaphoreA);
	if(begin == 0)
		begin = clock();

	if(_window != nullptr) {
		_window->UpdateWindow();

		auto res = VK_SUCCESS;

		clock_t end = clock();

		if(double(end - begin) / CLOCKS_PER_SEC > 0.05) {
			//for(int i = 0; i < _meshNum; i++) {
			//	meshes [i]->RotateZ(3);
			//}

			//begin = clock();
		}

		InitRenderPass(_imageKHRindex);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipeline());

		const VkDeviceSize offsets [1] = { 0 };


		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_vertexBuffer, offsets);
		vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);
		VkRect2D rect2d;
		rect2d.extent.width = _winWidth;
		rect2d.extent.height = _winHeight;
		rect2d.offset.x = 0;
		rect2d.offset.y = 0;
		vkCmdSetScissor(_commandBuffer, 0, 1, &rect2d);


		for(int i = 0; i < _meshNum; i++) {

			const uint32_t offset = i * _uniformBufferSize;

			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetPipelineLayout(),
								0, 1, _descriptorSets.data(), 1, &offset);

			vkCmdDraw(_commandBuffer, vertices.size(), 1, 0, 0);
		}

		vkCmdEndRenderPass(_commandBuffer);

		prePresentBarrier.image = GetSwapchainImage() [_imageKHRindex];

		vkCmdPipelineBarrier(_commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
							 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

		const VkCommandBuffer cmdBufs [] = { _commandBuffer };


		VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		vkResetFences(_deviceHandler, 1, &_drawFence);

		VkSubmitInfo submitInfo [1] = {};
		submitInfo [0].pNext = NULL;
		submitInfo [0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo [0].waitSemaphoreCount = 1;
		submitInfo [0].pWaitSemaphores = &_semaphoreA;
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
		present.pSwapchains = pGetVulcanSwapchainKHR();
		present.pImageIndices = &_imageKHRindex;

		do {
			res = vkWaitForFences(_deviceHandler, 1, &_drawFence, VK_TRUE, UINT64_MAX);
		} while(res == VK_TIMEOUT);

		assert(res == VK_SUCCESS);
		ErrorCheck(vkQueuePresentKHR(_queue, &present));

		ExecuteBeginCommandBuffer();

		return _window->isOpened();
	}

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

void VulkanRenderer::InitSemaphore(VkSemaphore* semaphore) {
	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo {};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ErrorCheck(vkCreateSemaphore(_deviceHandler, &presentCompleteSemaphoreCreateInfo, nullptr, semaphore));

	ErrorCheck(vkAcquireNextImageKHR(_deviceHandler, GetVulcanSwapchainKHR(), UINT64_MAX, *semaphore, VK_NULL_HANDLE, &_imageKHRindex));

	set_image_layout(GetSwapchainImage() [_imageKHRindex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
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
	//buffferCreateInfo.size =  _meshNum * _vertexPerMesh * sizeof(Vertex);
	buffferCreateInfo.size = vertices.size() * sizeof(Vertex);
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

	uint8_t *pData;
	ErrorCheck(vkMapMemory(_deviceHandler, _vertexBufferMemory, 0, _memoryReqs.size, 0, (void **) &pData));

	//memcpy(pData, vertices.data(), _meshNum * _vertexPerMesh* sizeof(Vertex));
	memcpy(pData, vertices.data(), vertices.size() * sizeof(Vertex));
	vkUnmapMemory(_deviceHandler, _vertexBufferMemory);

	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;

	ErrorCheck(vkCreateFence(_deviceHandler, &fenceInfo, nullptr, &_drawFence));
	
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
	//prePresentBarrier.image = _window->GetVulkanWindow()->GetSwapchainImage() [_imageKHRindex];

	//InitSemaphore(&_semaphoreA);
}

void VulkanRenderer::DeinitVertexBuffer() {
	vkDestroySemaphore(_deviceHandler, _semaphoreA, nullptr);
	vkDestroyFence(_deviceHandler, _drawFence, nullptr);
	vkDestroyBuffer(_deviceHandler, _vertexBuffer, nullptr);
	vkFreeMemory(_deviceHandler, _vertexBufferMemory, nullptr);
}

void VulkanRenderer::Draw(uint32_t &imageKHRindex, VkSemaphore* semaphore) {
	//vkCmdDraw(_commandBuffer, _meshNum * _vertexPerMesh, 1, 0, 0);

	vkCmdDraw(_commandBuffer, vertices.size(), 1, 0, 0);
}


void VulkanRenderer::initUniformBuffer() {
	//Create uniform buffer
	_uniformBufferSize = (sizeof(_mvp) + 255) & ~255;

	VkBufferCreateInfo uniformBufferCreateInfo {};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.size = _uniformBufferSize * _meshNum;
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uniformBufferCreateInfo.flags = 0;
	ErrorCheck(vkCreateBuffer(_deviceHandler, &uniformBufferCreateInfo, NULL, &_uniformDataBuffer));

	VkMemoryAllocateInfo memoryAllocatorInfo {};
	memoryAllocatorInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	vkGetBufferMemoryRequirements(_deviceHandler, _uniformDataBuffer, &_uniformMemoryRequirements);

	memoryAllocatorInfo.allocationSize = _uniformMemoryRequirements.size;

	uint32_t memoryTypeCount = _memoryProperties.memoryTypeCount;
	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((_uniformMemoryRequirements.memoryTypeBits & 1) == 1) {
			if((_memoryProperties.memoryTypes [i].propertyFlags &
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				memoryAllocatorInfo.memoryTypeIndex = i;
			}
		}
		_uniformMemoryRequirements.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(_deviceHandler, &memoryAllocatorInfo, nullptr, &_uniformBufferMemory));

	ErrorCheck(vkBindBufferMemory(_deviceHandler, _uniformDataBuffer, _uniformBufferMemory, 0));

	_uniformDescriptorInfo.buffer = _uniformDataBuffer;
	_uniformDescriptorInfo.offset = 0;
	_uniformDescriptorInfo.range = _uniformBufferSize;
}

void VulkanRenderer::deinitUniformBuffer() {
	vkDestroyBuffer(_deviceHandler, _uniformDataBuffer, nullptr);
	vkFreeMemory(_deviceHandler, _uniformBufferMemory, nullptr);
}

void VulkanRenderer::updateCamera() {
	_view = glm::lookAt(vulkanCamera.cameraPosition,//Camera Position,
						vulkanCamera.lookAt, // look at
						glm::vec3(0, -1, 0) /*head up */);

	vkGetBufferMemoryRequirements(_deviceHandler, _uniformDataBuffer, &_uniformMemoryRequirements);

	VkMemoryAllocateInfo memoryAllocatorInfo {};
	memoryAllocatorInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	memoryAllocatorInfo.allocationSize = _uniformMemoryRequirements.size;

	uint32_t memoryTypeCount = _memoryProperties.memoryTypeCount;

	uint8_t *pData;
	ErrorCheck(vkMapMemory(_deviceHandler, _uniformBufferMemory, 0, _uniformMemoryRequirements.size, 0, (void **) &pData));
	for(int i = 0; i < _meshNum; i++) {
		glm::mat4x4 model = glm::translate(_model, glm::vec3(_translateValues [i].X, _translateValues [i].Y, _translateValues [i].Z));
		_mvp = _clip * _projection * _view * model;
		memcpy(pData + i * _uniformBufferSize, &_mvp, sizeof(_mvp));
	}


	vkUnmapMemory(_deviceHandler, _uniformBufferMemory);
}

VkViewport* VulkanRenderer::GetViewport() {
	return &_viewport;
}

void VulkanRenderer::initDescriptorSet() {
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo [1];
	descriptorSetAllocateInfo [0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo [0].pNext = NULL;
	descriptorSetAllocateInfo [0].descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo [0].descriptorSetCount = 1;
	descriptorSetAllocateInfo [0].pSetLayouts = _descriptorSetLayout.data();

	_descriptorSets.resize(1);

	ErrorCheck(vkAllocateDescriptorSets(_deviceHandler, descriptorSetAllocateInfo, _descriptorSets.data()));

	VkWriteDescriptorSet writes [1];

	writes [0] = {};
	writes [0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes [0].pNext = NULL;
	writes [0].dstSet = _descriptorSets [0];
	writes [0].descriptorCount = 1;
	writes [0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writes [0].pBufferInfo = &_uniformDescriptorInfo;
	writes [0].dstArrayElement = 0;
	writes [0].dstBinding = 0;

	vkUpdateDescriptorSets(_deviceHandler,  1, writes, 0, nullptr);

}

void VulkanRenderer::initDescriptorPool() {
	VkDescriptorPoolSize typeCount [2];
	typeCount [0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	typeCount [0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount =  1;
	descriptorPoolCreateInfo.pPoolSizes = typeCount;

	ErrorCheck(vkCreateDescriptorPool(_deviceHandler, &descriptorPoolCreateInfo, nullptr, &_descriptorPool));
}

void VulkanRenderer::deinitDescriptorPool() {
	vkDestroyDescriptorPool(_deviceHandler, _descriptorPool, nullptr);
}

const std::vector<VkDescriptorSet> VulkanRenderer::GetDescriptorSet() const {
	return _descriptorSets;
}

void VulkanRenderer::initDescriptorAndPipelineLayout() {
	VkDescriptorSetLayoutBinding layoutBindings [2];
	layoutBindings [0].binding = 0;
	layoutBindings [0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	layoutBindings [0].descriptorCount = 1;
	layoutBindings [0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings [0].pImmutableSamplers = NULL;

	uint32_t descriptorLayoutCount =  1;
	VkDescriptorSetLayoutCreateInfo descriptorLayout {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.bindingCount = descriptorLayoutCount;
	descriptorLayout.pBindings = layoutBindings;

	_descriptorSetLayout.resize(_numDescriptorSets);
	ErrorCheck(vkCreateDescriptorSetLayout(_deviceHandler, &descriptorLayout, nullptr, _descriptorSetLayout.data()));

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = _numDescriptorSets;
	pipelineLayoutCreateInfo.pSetLayouts = _descriptorSetLayout.data();

	ErrorCheck(vkCreatePipelineLayout(_deviceHandler, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout));
}

void VulkanRenderer::deinitDescriptorAndPipelineLayout() {
	for(uint32_t i = 0; i < _numDescriptorSets; i++) {
		vkDestroyDescriptorSetLayout(_deviceHandler, _descriptorSetLayout [i], nullptr);
	}

	vkDestroyPipelineLayout(_deviceHandler, _pipelineLayout, nullptr);
}

const VkPipelineLayout VulkanRenderer::GetPipelineLayout() const {
	return _pipelineLayout;
}

void VulkanRenderer::initPipeline() {

	std::array<VkDynamicState, 2> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {};

	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateEnables.size();

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {};
	memset(&pipelineVertexInputStateCreateInfo, 0, sizeof(pipelineVertexInputStateCreateInfo));
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &_vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = _vertexInputAttributeDescription;

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_TRUE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0;
	pipelineRasterizationStateCreateInfo.depthBiasClamp = 0;
	pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState [1];
	pipelineColorBlendAttachmentState [0].colorWriteMask = 0xf;
	pipelineColorBlendAttachmentState [0].blendEnable = VK_FALSE;
	pipelineColorBlendAttachmentState [0].alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState [0].colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState [0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState [0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState [0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState [0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	pipelineColorBlendStateCreateInfo.blendConstants [0] = 1.0f;
	pipelineColorBlendStateCreateInfo.blendConstants [1] = 1.0f;
	pipelineColorBlendStateCreateInfo.blendConstants [2] = 1.0f;
	pipelineColorBlendStateCreateInfo.blendConstants [3] = 1.0f;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;

	//dynamicStateEnables [pipelineDynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;

	pipelineViewportStateCreateInfo.scissorCount = 1;

	//dynamicStateEnables [pipelineDynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.minDepthBounds = 0;
	pipelineDepthStencilStateCreateInfo.maxDepthBounds = 0;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.back.compareMask = 0;
	pipelineDepthStencilStateCreateInfo.back.reference = 0;
	pipelineDepthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.writeMask = 0;
	pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.minSampleShading = 0.0;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout = _pipelineLayout;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pStages = _shaderStagesCreateInfo;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.renderPass = _renderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	ErrorCheck(vkCreateGraphicsPipelines(GetVulcanDevice(), _pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &_pipeline));
}

const VkSwapchainKHR VulkanRenderer::GetVulcanSwapchainKHR() const {
	return _swapchain;
}

const VkSwapchainKHR * VulkanRenderer::pGetVulcanSwapchainKHR() const {
	return &_swapchain;
}

std::vector<VkImage> VulkanRenderer::GetSwapchainImage() const {
	return _swapchainImages;
}

const VkRenderPass VulkanRenderer::GetRenderPass() const {
	return _renderPass;
}

const std::vector<VkFramebuffer> VulkanRenderer::GetFrameBuffer() const {
	return _frameBuffers;
}

const VkPipeline VulkanRenderer::GetPipeline() const {
	return _pipeline;
}

void VulkanRenderer::InitRenderPass(uint32_t &imageKHRindex) {

	VkClearValue clear_values [2];
	clear_values [0].color.float32 [0] = 0.2f;
	clear_values [0].color.float32 [1] = 0.2f;
	clear_values [0].color.float32 [2] = 0.2f;
	clear_values [0].color.float32 [3] = 0.2f;
	clear_values [1].depthStencil.depth = 1.0f;
	clear_values [1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo renderPassBeginInfo {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.framebuffer = _frameBuffers [imageKHRindex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = _surfaceSizeX;
	renderPassBeginInfo.renderArea.extent.height = _surfaceSizeY;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clear_values;

	vkCmdBeginRenderPass(GetVulcanCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::initVulcanSurface() {
	assert(_win32Window, "Window is not created!");
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = _win32Instance;
	surfaceCreateInfo.hwnd = _win32Window;

	ErrorCheck(vkCreateWin32SurfaceKHR(GetVulcanInstance(), &surfaceCreateInfo, nullptr, &_surface));

	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(GetVulcanPhysicalDevice(), GetVulcanGraphicsQueueFamilyIndex(), _surface, &WSI_supported); // witout that check it will crash (???WTF)
	if(!WSI_supported) {
		assert(0 && "WSI not supported");
		std::exit(-1);
	}

	//query surface
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetVulcanPhysicalDevice(), _surface, &_surfaceCapabilitesKHR);
	if(_surfaceCapabilitesKHR.currentExtent.width < UINT32_MAX - 1) {
		_surfaceSizeX = _surfaceCapabilitesKHR.currentExtent.width;
		_surfaceSizeY = _surfaceCapabilitesKHR.currentExtent.height;
	}

	{
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulcanPhysicalDevice(), _surface, &formatCount, nullptr);
		assert(formatCount > 0, "formatCount < 0 !");
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulcanPhysicalDevice(), _surface, &formatCount, formats.data());

		if(formats [0].format == VK_FORMAT_UNDEFINED) {
			_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		} else {
			_surfaceFormat = formats [0];
		}
	}
}

void VulkanRenderer::deinitVulcanSurface() {
	vkDestroySurfaceKHR(GetVulcanInstance(), _surface, nullptr);
}

void VulkanRenderer::initSwapchain() {

	if(_swapchainImageCount > _surfaceCapabilitesKHR.maxImageCount) _swapchainImageCount = _surfaceCapabilitesKHR.maxImageCount;
	if(_swapchainImageCount < _surfaceCapabilitesKHR.minImageCount + 1) _swapchainImageCount = _surfaceCapabilitesKHR.minImageCount + 1;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulcanPhysicalDevice(), _surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulcanPhysicalDevice(), _surface, &presentModeCount, presentModes.data());

		for(auto mode : presentModes) {
			if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = mode;
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = _surface;
	swapchainCreateInfo.minImageCount = _swapchainImageCount;
	swapchainCreateInfo.imageFormat = _surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = _surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent.width = _surfaceSizeX;
	swapchainCreateInfo.imageExtent.height = _surfaceSizeY;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;			//ignored when VK_SHARING_MODE_EXCLUSIVE
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;		//ignored when VK_SHARING_MODE_EXCLUSIVE
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; //future consider set it to resize window

	ErrorCheck(vkCreateSwapchainKHR(GetVulcanDevice(), &swapchainCreateInfo, nullptr, &_swapchain));

	ErrorCheck(vkGetSwapchainImagesKHR(GetVulcanDevice(), _swapchain, &_swapchainImageCount, nullptr));
}

void VulkanRenderer::deinitSwapchain() {
	vkDestroySwapchainKHR(GetVulcanDevice(), _swapchain, nullptr);
}

void VulkanRenderer::initSwapchainImages() {
	_swapchainImages.resize(_swapchainImageCount);
	_swapchainImageView.resize(_swapchainImageCount);

	ErrorCheck(vkGetSwapchainImagesKHR(GetVulcanDevice(), _swapchain, &_swapchainImageCount, _swapchainImages.data()));
	for(uint32_t i = 0; i < _swapchainImageCount; ++i) {
		VkImageViewCreateInfo imageViewCreateInfo {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = _swapchainImages [i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _surfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(GetVulcanDevice(), &imageViewCreateInfo, nullptr, &_swapchainImageView [i]);
	}
}

void VulkanRenderer::deinitSwapchainImages() {
	for(auto view : _swapchainImageView) {
		vkDestroyImageView(GetVulcanDevice(), view, nullptr);
	}
}

void VulkanRenderer::initDepthBuffer() {

	VkFormatProperties formatPropeties;

	VkImageCreateInfo imageCreateInfo {};

	vkGetPhysicalDeviceFormatProperties(GetVulcanPhysicalDevice(), _depthFormat, &formatPropeties);
	if(formatPropeties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	} else if(formatPropeties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	} else {
		assert(0 && "VK_FORMAT_D16_UNORM Unsupported.\n");
		exit(-1);
	}

	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.format = _depthFormat;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = _surfaceSizeX;
	imageCreateInfo.extent.height = _surfaceSizeY;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = NULL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.flags = 0;

	VkImageViewCreateInfo imageViewCreateInfo {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = VK_NULL_HANDLE;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = _depthFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	if(_depthFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
	   _depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
	   _depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkMemoryAllocateInfo memoryAllocatorInfo {};
	memoryAllocatorInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryRequirements memoryRequirements;

	ErrorCheck(vkCreateImage(GetVulcanDevice(), &imageCreateInfo, nullptr, &_depthImage));

	vkGetImageMemoryRequirements(GetVulcanDevice(), _depthImage, &memoryRequirements);
	memoryAllocatorInfo.allocationSize = memoryRequirements.size;
	uint32_t memoryTypeCount = GetVulcanPhysicalDeviceMemoryProperties().memoryTypeCount;

	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((memoryRequirements.memoryTypeBits & 1) == 1) {
			if((GetVulcanPhysicalDeviceMemoryProperties().memoryTypes [i].propertyFlags & 0) == 0) {
				memoryAllocatorInfo.memoryTypeIndex = i;
			}
		}
		memoryRequirements.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(GetVulcanDevice(), &memoryAllocatorInfo, nullptr, &_depthMemory));
	ErrorCheck(vkBindImageMemory(GetVulcanDevice(), _depthImage, _depthMemory, 0));
	setImageLayout();

	imageViewCreateInfo.image = _depthImage;
	ErrorCheck(vkCreateImageView(GetVulcanDevice(), &imageViewCreateInfo, nullptr, &_depthImageView));
}

void VulkanRenderer::deinitDepthBuffer() {
	vkDestroyImageView(GetVulcanDevice(), _depthImageView, nullptr);
	vkDestroyImage(GetVulcanDevice(), _depthImage, nullptr);
	vkFreeMemory(GetVulcanDevice(), _depthMemory, nullptr);
}

void VulkanRenderer::setImageLayout() {
	VkImageLayout oldImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout newImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageMemoryBarrier imageMemoryBarrier {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = 0;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = _depthImage;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	if(oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if(oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if(oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	}

	if(newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask =
			VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if(newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if(newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask =
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(GetVulcanCommandBuffer(), srcStages, destStages, 0, 0, NULL, 0, NULL,
						 1, &imageMemoryBarrier);
}

void VulkanRenderer::initRenderpass() {
	VkAttachmentDescription attachments [2];
	attachments [0].format = _surfaceFormat.format;
	attachments [0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments [0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments [0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments [0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments [0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments [0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments [0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments [0].flags = 0;

	attachments [1].format = _depthFormat;
	attachments [1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments [1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments [1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments [1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments [1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments [1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments [1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments [1].flags = 0;

	VkAttachmentReference colorReference {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pDepthStencilAttachment = &depthReference;

	VkRenderPassCreateInfo renderPassCreateInfo {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = attachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;

	ErrorCheck(vkCreateRenderPass(GetVulcanDevice(), &renderPassCreateInfo, nullptr, &_renderPass));
}

void VulkanRenderer::deinitRenderpass() {
	vkDestroyRenderPass(GetVulcanDevice(), _renderPass, nullptr);
}

void VulkanRenderer::initShaders() {

	glslang::InitializeProcess();
	VkShaderModuleCreateInfo moduleCreateInfo;

	std::vector<unsigned int> vtx_spv;
	_shaderStagesCreateInfo [0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	_shaderStagesCreateInfo [0].pNext = NULL;
	_shaderStagesCreateInfo [0].pSpecializationInfo = NULL;
	_shaderStagesCreateInfo [0].flags = 0;
	_shaderStagesCreateInfo [0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	_shaderStagesCreateInfo [0].pName = "main";

	bool ret = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
	assert(ret);

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
	moduleCreateInfo.pCode = vtx_spv.data();

	ErrorCheck(vkCreateShaderModule(GetVulcanDevice(), &moduleCreateInfo, NULL, &_shaderStagesCreateInfo [0].module));

	std::vector<unsigned int> frag_spv;
	_shaderStagesCreateInfo [1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	_shaderStagesCreateInfo [1].pNext = NULL;
	_shaderStagesCreateInfo [1].pSpecializationInfo = NULL;
	_shaderStagesCreateInfo [1].flags = 0;
	_shaderStagesCreateInfo [1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	_shaderStagesCreateInfo [1].pName = "main";

	ret = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
	assert(ret);

	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
	moduleCreateInfo.pCode = frag_spv.data();

	vkCreateShaderModule(GetVulcanDevice(), &moduleCreateInfo, NULL, &_shaderStagesCreateInfo [1].module);

	glslang::FinalizeProcess();
}

void VulkanRenderer::deinitShaders() {
	vkDestroyShaderModule(GetVulcanDevice(), _shaderStagesCreateInfo [0].module, nullptr);
	vkDestroyShaderModule(GetVulcanDevice(), _shaderStagesCreateInfo [1].module, nullptr);
}

void VulkanRenderer::initframeBuffer() {
	VkImageView attachments [2];
	attachments [1] = _depthImageView;

	VkFramebufferCreateInfo framebufferCreateInfo {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = _renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = attachments;
	framebufferCreateInfo.width = _surfaceSizeX;
	framebufferCreateInfo.height = _surfaceSizeY;
	framebufferCreateInfo.layers = 1;

	_frameBuffers.resize(_swapchainImageCount);

	for(uint32_t i = 0; i < _swapchainImageCount; i++) {
		attachments [0] = _swapchainImageView [i];
		vkCreateFramebuffer(GetVulcanDevice(), &framebufferCreateInfo, nullptr, &_frameBuffers [i]);
	}
}

void VulkanRenderer::deinitframeBuffer() {
	for(uint32_t i = 0; i < _swapchainImageCount; i++) {
		vkDestroyFramebuffer(GetVulcanDevice(), _frameBuffers [i], nullptr);
	}

}

void VulkanRenderer::initPipelineCache() {
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	ErrorCheck(vkCreatePipelineCache(GetVulcanDevice(), &pipelineCacheCreateInfo, nullptr, &_pipelineCache));
}

void VulkanRenderer::deinitPipelineCache() {
	vkDestroyPipelineCache(GetVulcanDevice(), _pipelineCache, nullptr);
}

static uint32_t vertex_stride() {
	// Position + Normal
	const int comp_count = 8;

	return sizeof(float) * comp_count;
}

void VulkanRenderer::initVertexBuffer() {

	_vertexInputBindingDescription.binding = 0;
	_vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	_vertexInputBindingDescription.stride = vertex_stride();

	_vertexInputAttributeDescription [0].binding = 0;
	_vertexInputAttributeDescription [0].location = 0;
	_vertexInputAttributeDescription [0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_vertexInputAttributeDescription [0].offset = 0;
	_vertexInputAttributeDescription [1].binding = 0;
	_vertexInputAttributeDescription [1].location = 1;
	_vertexInputAttributeDescription [1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_vertexInputAttributeDescription [1].offset = 16;

}

void VulkanRenderer::deinitPipeline() {
	vkDestroyPipeline(GetVulcanDevice(), _pipeline, nullptr);
}
