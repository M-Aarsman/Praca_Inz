#include "Window.h"
#include "VulcanRenderer.h"
#include "Shaders.h"

#include <assert.h>
#include <iostream>
#include <SPIRV\GlslangToSpv.h>

LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch(uMsg) {
		case WM_CLOSE:
		window->Close();
		return 0;
		case WM_SIZE:
		break;
		default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}





/* <<<<<<<<<<<<<<<TO SHADER.H /.CPP*/
EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
	switch(shader_type) {
		case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

		case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

		case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

		case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

		default:
		return EShLangVertex;
	}
}

void init_resources(TBuiltInResource &Resources) {
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *shaderText,
			   std::vector<unsigned int> &spirv) {

	EShLanguage stage = FindLanguage(shaderType);
	glslang::TShader shader(stage);
	glslang::TProgram program;
	const char *shaderStrings [1];
	TBuiltInResource Resources;
	init_resources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);

	shaderStrings [0] = shaderText;
	shader.setStrings(shaderStrings, 1);

	if(!shader.parse(&Resources, 100, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		return false; // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if(!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
	return true;
}


/* <<<<<<<<<<<<<<<TO SHADER.H /.CPP*/
struct Vertex {
	float posX, posY, posZ, posW; // Position data
	float r, g, b, a;             // Color
};

struct VertexUV {
	float posX, posY, posZ, posW; // Position data
	float u, v;                   // texture u,v
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_) (_u_), (_v_)

static const Vertex g_vb_solid_face_colors_Data [] = {
	{ XYZ1(-1, -1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, -1, -1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 0.f) },

	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, 1, 1), XYZ1(0.f, 1.f, 0.f) },

	{ XYZ1(1, 1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 0.f, 1.f) },

	{ XYZ1(-1, 1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(-1, -1, -1), XYZ1(1.f, 1.f, 0.f) },

	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f) },

	{ XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
};


Window::Window(VulcanRenderer* renderer, uint32_t sizeX, uint32_t sizeY, const char* name) {
	assert(renderer != nullptr, "Renderer is nullptr!");

	_renderer = renderer;
	_win32ClassName = "ApiWindow";
	_win32Instance = GetModuleHandle(nullptr);
	_sizeX = _surfaceSizeX = sizeX;
	_sizeY = _surfaceSizeY =sizeY;
	_winTitle = name;

	initOSWindow();
	initVulcanSurface();
}

void Window::InitWindow() {
	initSwapchain();
	initSwapchainImages();
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
	_opened = true;
}

void Window::Close() {
	if(_opened) {
		deinitPipeline();
		deinitPipelineCache();
		deinitDescriptorPool();
		deinitVertexBuffer();
		deinitframeBuffer();
		deinitShaders();
		deinitRenderpass();
		deinitDescriptorAndPipelineLayout();
		deinitUniformBuffer();
		deinitDepthBuffer();
		deinitSwapchainImages();
		deinitSwapchain();
		deinitVulcanSurface();
		DestroyWindow(_win32Window);
		UnregisterClass(_win32ClassName, _win32Instance);
		_opened = false;
	}
}


void Window::UpdateWindow() {
	MSG msg;
	if(PeekMessage(&msg, _win32Window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool Window::isOpened() {
	return _opened;
}

const VkSwapchainKHR Window::GetVulcanSwapchainKHR() const {
	return _swapchain;
}

const VkSwapchainKHR * Window::pGetVulcanSwapchainKHR() const {
	return &_swapchain;
}

std::vector<VkImage> Window::GetSwapchainImage() const {
	return _swapchainImages;
}

const VkRenderPass Window::GetRenderPass() const {
	return _renderPass;
}

const std::vector<VkFramebuffer> Window::GetFrameBuffer() const {
	return _frameBuffers;
}

const uint32_t Window::GetWidth() const {
	return _sizeX;
}

const uint32_t Window::GetHeight() const {
	return _sizeY;
}

const VkPipeline Window::GetPipeline() const {
	return _pipeline;
}

const VkPipelineLayout Window::GetPipelineLayout() const {
	return _pipelineLayout;
}

const std::vector<VkDescriptorSet> Window::GetDescriptorSet() const {
	return _descriptorSets;
}

const VkBuffer* Window::GetVertexBuffer() {
	return &_vertexBuffer;
}

void Window::initOSWindow() {
	WNDCLASSEX winClass {};
	winClass.cbSize = sizeof(winClass);
	winClass.style = CS_VREDRAW | CS_HREDRAW;
	winClass.lpfnWndProc = WindowsEventHandler;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = _win32Instance;
	winClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName = NULL;
	winClass.lpszClassName = _win32ClassName;
	winClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if(!RegisterClassEx(&winClass)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT rect = { 0, 0, LONG(_sizeX), LONG(_sizeY) };

	AdjustWindowRectEx(&rect, style, FALSE, ex_style);

	_win32Window = CreateWindowEx(0,
								  winClass.lpszClassName,
								  (LPCSTR) _winTitle, style,
								  CW_USEDEFAULT,
								  CW_USEDEFAULT,
								  rect.right - rect.left,
								  rect.bottom - rect.top,
								  NULL,
								  NULL,
								  _win32Instance,
								  NULL);

	if(!_win32Window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(_win32Window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(_win32Window, SW_SHOW);
	SetForegroundWindow(_win32Window);
	SetFocus(_win32Window);
}

void Window::initVulcanSurface() {
	assert(_win32Window, "Window is not created!");
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = _win32Instance;
	surfaceCreateInfo.hwnd = _win32Window;

	ErrorCheck(vkCreateWin32SurfaceKHR(_renderer->GetVulcanInstance(), &surfaceCreateInfo, nullptr, &_surface));
	
	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(_renderer->GetVulcanPhysicalDevice(), _renderer->GetVulcanGraphicsQueueFamilyIndex(), _surface, &WSI_supported); // witout that check it will crash (???WTF)
	if(!WSI_supported) {
		assert(0 && "WSI not supported");
		std::exit(-1);
	}
	
	//query surface
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_renderer->GetVulcanPhysicalDevice(), _surface, &_surfaceCapabilitesKHR);
	if(_surfaceCapabilitesKHR.currentExtent.width < UINT32_MAX - 1) {
		_surfaceSizeX = _surfaceCapabilitesKHR.currentExtent.width;
		_surfaceSizeY = _surfaceCapabilitesKHR.currentExtent.height;
	}

	{
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_renderer->GetVulcanPhysicalDevice(), _surface, &formatCount,nullptr);
		assert(formatCount > 0, "formatCount < 0 !");
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_renderer->GetVulcanPhysicalDevice(), _surface, &formatCount, formats.data());

		if(formats [0].format == VK_FORMAT_UNDEFINED) {
			_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
			_surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		} 
		else {
			_surfaceFormat = formats [0];
		}
	}
}

void Window::deinitVulcanSurface() {
	vkDestroySurfaceKHR(_renderer->GetVulcanInstance(), _surface, nullptr);
}

void Window::initSwapchain() {

	if(_swapchainImageCount > _surfaceCapabilitesKHR.maxImageCount) _swapchainImageCount = _surfaceCapabilitesKHR.maxImageCount;
	if(_swapchainImageCount < _surfaceCapabilitesKHR.minImageCount + 1) _swapchainImageCount = _surfaceCapabilitesKHR.minImageCount + 1;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->GetVulcanPhysicalDevice(), _surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->GetVulcanPhysicalDevice(), _surface, &presentModeCount, presentModes.data());

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

	ErrorCheck(vkCreateSwapchainKHR(_renderer->GetVulcanDevice(), &swapchainCreateInfo, nullptr, &_swapchain));

	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->GetVulcanDevice(), _swapchain, &_swapchainImageCount, nullptr));
}

void Window::deinitSwapchain() {
	vkDestroySwapchainKHR(_renderer->GetVulcanDevice(), _swapchain, nullptr);
}

void Window::initSwapchainImages() {
	_swapchainImages.resize(_swapchainImageCount);
	_swapchainImageView.resize(_swapchainImageCount);

	ErrorCheck(vkGetSwapchainImagesKHR(_renderer->GetVulcanDevice(), _swapchain, &_swapchainImageCount, _swapchainImages.data()));
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

		vkCreateImageView(_renderer->GetVulcanDevice(), &imageViewCreateInfo, nullptr, &_swapchainImageView [i]);
	}
}

void Window::deinitSwapchainImages() {
	for(auto view : _swapchainImageView) {
		vkDestroyImageView(_renderer->GetVulcanDevice(), view, nullptr);
	}
}

void Window::initDepthBuffer() {

	VkFormatProperties formatPropeties;

	VkImageCreateInfo imageCreateInfo {};

	vkGetPhysicalDeviceFormatProperties(_renderer->GetVulcanPhysicalDevice(), _depthFormat, &formatPropeties);
	if(formatPropeties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	} else if(formatPropeties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	} else {
		assert( 0 && "VK_FORMAT_D16_UNORM Unsupported.\n");
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

	ErrorCheck(vkCreateImage(_renderer->GetVulcanDevice(), &imageCreateInfo, nullptr, &_depthImage));

	vkGetImageMemoryRequirements(_renderer->GetVulcanDevice(), _depthImage, &memoryRequirements);
	memoryAllocatorInfo.allocationSize = memoryRequirements.size;
	uint32_t memoryTypeCount = _renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypeCount;

	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((memoryRequirements.memoryTypeBits & 1) == 1) {
			if((_renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypes [i].propertyFlags & 0) == 0) {
				memoryAllocatorInfo.memoryTypeIndex = i;
			}
		}
		memoryRequirements.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(_renderer->GetVulcanDevice(), &memoryAllocatorInfo, nullptr, &_depthMemory));
	ErrorCheck(vkBindImageMemory(_renderer->GetVulcanDevice(), _depthImage, _depthMemory, 0));
	setImageLayout();

	imageViewCreateInfo.image = _depthImage;
	ErrorCheck(vkCreateImageView(_renderer->GetVulcanDevice(), &imageViewCreateInfo, nullptr, &_depthImageView));
}

void Window::deinitDepthBuffer() {
	vkDestroyImageView(_renderer->GetVulcanDevice(), _depthImageView, nullptr);
	vkDestroyImage(_renderer->GetVulcanDevice(), _depthImage, nullptr);
	vkFreeMemory(_renderer->GetVulcanDevice(), _depthMemory, nullptr);
}

void Window::setImageLayout() {
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

	vkCmdPipelineBarrier(_renderer->GetVulcanCommandBuffer(), srcStages, destStages, 0, 0, NULL, 0, NULL,
						 1, &imageMemoryBarrier);
}

void Window::initUniformBuffer() {

	float fov = glm::radians(45.0f);
	if(_sizeX > _sizeY) {
		fov *= static_cast<float>(_sizeY) / static_cast<float>(_sizeX);
	}

	_projection = glm::perspective(fov, static_cast<float>(_sizeX) / static_cast<float>(_sizeY), 0.1f, 100.0f);

	_view = glm::lookAt(glm::vec3(5, 3, 10),//Camera Position,
								 glm::vec3(0, 0, 0), // look at
								 glm::vec3(0, -1, 0) /*head up */);
	_model = glm::mat4(1.0f);

	// Vulkan clip space has inverted Y and half Z.
	_clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, -1.0f, 0.0f, 0.0f,
					  0.0f, 0.0f, 0.5f, 0.0f,
					  0.0f, 0.0f, 0.5f, 1.0f);	

	_mvp = _clip * _projection * _view * _model;

	//Create uniform buffer

	VkBufferCreateInfo uniformBufferCreateInfo {};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.size = sizeof(_mvp);
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uniformBufferCreateInfo.flags = 0;
	ErrorCheck(vkCreateBuffer(_renderer->GetVulcanDevice(), &uniformBufferCreateInfo, NULL, &_uniformDataBuffer));

	VkMemoryAllocateInfo memoryAllocatorInfo {};
	memoryAllocatorInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_renderer->GetVulcanDevice(), _uniformDataBuffer, &memoryRequirements);

	memoryAllocatorInfo.allocationSize = memoryRequirements.size;

	uint32_t memoryTypeCount = _renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypeCount;
	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((memoryRequirements.memoryTypeBits & 1) == 1) {
			if((_renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypes [i].propertyFlags & 
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				memoryAllocatorInfo.memoryTypeIndex = i;
			}
		}
		memoryRequirements.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(_renderer->GetVulcanDevice(), &memoryAllocatorInfo, nullptr, &_uniformBufferMemory));

	uint8_t *pData;
	ErrorCheck(vkMapMemory(_renderer->GetVulcanDevice(), _uniformBufferMemory, 0, memoryRequirements.size, 0, (void **) &pData));
	memcpy(pData, &_mvp, sizeof(_mvp));
	vkUnmapMemory(_renderer->GetVulcanDevice(), _uniformBufferMemory);

	ErrorCheck(vkBindBufferMemory(_renderer->GetVulcanDevice(), _uniformDataBuffer, _uniformBufferMemory, 0));

	_uniformDescriptorInfo.buffer = _uniformDataBuffer;
	_uniformDescriptorInfo.offset = 0;
	_uniformDescriptorInfo.range = sizeof(_mvp);
}

void Window::deinitUniformBuffer() {
	vkDestroyBuffer(_renderer->GetVulcanDevice(), _uniformDataBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulcanDevice(), _uniformBufferMemory, nullptr);
}

void Window::initDescriptorAndPipelineLayout() {
	VkDescriptorSetLayoutBinding layoutBindings [2];
	layoutBindings [0].binding = 0;
	layoutBindings [0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings [0].descriptorCount = 1;
	layoutBindings [0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings [0].pImmutableSamplers = NULL;

	if(_useTexture) {
		layoutBindings [1].binding = 1;
		layoutBindings [1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings [1].descriptorCount = 1;
		layoutBindings [1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings [1].pImmutableSamplers = NULL;
	}

	uint32_t descriptorLayoutCount = _useTexture ? 2 : 1;
	VkDescriptorSetLayoutCreateInfo descriptorLayout {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.bindingCount = descriptorLayoutCount;
	descriptorLayout.pBindings = layoutBindings;

	_descriptorSetLayout.resize(_numDescriptorSets);
	ErrorCheck(vkCreateDescriptorSetLayout(_renderer->GetVulcanDevice(), &descriptorLayout, nullptr, _descriptorSetLayout.data()));

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = _numDescriptorSets;
	pipelineLayoutCreateInfo.pSetLayouts = _descriptorSetLayout.data();

	ErrorCheck(vkCreatePipelineLayout(_renderer->GetVulcanDevice(), &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout));
}

void Window::deinitDescriptorAndPipelineLayout() {
	for(uint32_t i = 0; i < _numDescriptorSets; i++) {
		vkDestroyDescriptorSetLayout(_renderer->GetVulcanDevice(), _descriptorSetLayout [i], nullptr);
	}

	vkDestroyPipelineLayout(_renderer->GetVulcanDevice(), _pipelineLayout, nullptr);
}

void Window::initRenderpass() {
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

	ErrorCheck(vkCreateRenderPass(_renderer->GetVulcanDevice(), &renderPassCreateInfo, nullptr, &_renderPass));
}

void Window::deinitRenderpass() {
	vkDestroyRenderPass(_renderer->GetVulcanDevice(), _renderPass, nullptr);
}

void Window::initShaders() {

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

	ErrorCheck(vkCreateShaderModule(_renderer->GetVulcanDevice(), &moduleCreateInfo, NULL, &_shaderStagesCreateInfo [0].module));

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

	vkCreateShaderModule(_renderer->GetVulcanDevice(), &moduleCreateInfo, NULL, &_shaderStagesCreateInfo [1].module);
	
	glslang::FinalizeProcess();
}

void Window::deinitShaders() {
	vkDestroyShaderModule(_renderer->GetVulcanDevice(), _shaderStagesCreateInfo [0].module, nullptr);
	vkDestroyShaderModule(_renderer->GetVulcanDevice(), _shaderStagesCreateInfo [1].module, nullptr);
}

void Window::initframeBuffer() {
	VkImageView attachments [2];
	attachments [1] = _depthImageView;

	VkFramebufferCreateInfo framebufferCreateInfo {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = _renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = attachments;
	framebufferCreateInfo.width = _sizeX;
	framebufferCreateInfo.height = _sizeX;
	framebufferCreateInfo.layers = 1;

	_frameBuffers.resize(_swapchainImageCount);

	for(uint32_t i = 0; i < _swapchainImageCount; i++) {
		attachments [0] = _swapchainImageView[i];
		vkCreateFramebuffer(_renderer->GetVulcanDevice(), &framebufferCreateInfo, nullptr, &_frameBuffers[i]);
	}
}

void Window::deinitframeBuffer() {
	for(uint32_t i = 0; i < _swapchainImageCount; i++) {
		vkDestroyFramebuffer(_renderer->GetVulcanDevice(), _frameBuffers [i], nullptr);
	}
	
}

void Window::initVertexBuffer() {
	VkBufferCreateInfo buffferCreateInfo {};
	buffferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffferCreateInfo.size = sizeof(g_vb_solid_face_colors_Data);
	buffferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ErrorCheck(vkCreateBuffer(_renderer->GetVulcanDevice(), &buffferCreateInfo, nullptr, &_vertexBuffer));

	VkMemoryRequirements memoryReqs;
	vkGetBufferMemoryRequirements(_renderer->GetVulcanDevice(), _vertexBuffer, &memoryReqs);

	VkMemoryAllocateInfo allocateInfo {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryReqs.size;

	uint32_t memoryTypeCount = _renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypeCount;
	for(uint32_t i = 0; i < memoryTypeCount; ++i) {
		if((memoryReqs.memoryTypeBits & 1) == 1) {
			if((_renderer->GetVulcanPhysicalDeviceMemoryProperties().memoryTypes [i].propertyFlags &
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				allocateInfo.memoryTypeIndex = i;
			}
		}
		memoryReqs.memoryTypeBits >>= 1;
	}

	ErrorCheck(vkAllocateMemory(_renderer->GetVulcanDevice(), &allocateInfo, nullptr, &_vertexBufferMemory));

	uint8_t *pData;
	ErrorCheck(vkMapMemory(_renderer->GetVulcanDevice(), _vertexBufferMemory, 0, memoryReqs.size, 0, (void **) &pData));

	memcpy(pData, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data));
	vkUnmapMemory(_renderer->GetVulcanDevice(), _vertexBufferMemory);
	ErrorCheck(vkBindBufferMemory(_renderer->GetVulcanDevice(), _vertexBuffer, _vertexBufferMemory, 0));

	_vertexInputBindingDescription.binding = 0;
	_vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	_vertexInputBindingDescription.stride = sizeof(g_vb_solid_face_colors_Data [0]);

	_vertexInputAttributeDescription [0].binding = 0;
	_vertexInputAttributeDescription [0].location = 0;
	_vertexInputAttributeDescription [0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_vertexInputAttributeDescription [0].offset = 0;
	_vertexInputAttributeDescription [1].binding = 0;
	_vertexInputAttributeDescription [1].location = 1;
	_vertexInputAttributeDescription [1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_vertexInputAttributeDescription [1].offset = 16;

}

void Window::deinitVertexBuffer() {
	vkDestroyBuffer(_renderer->GetVulcanDevice(), _vertexBuffer, nullptr);
	vkFreeMemory(_renderer->GetVulcanDevice(), _vertexBufferMemory, nullptr);
}

void Window::initDescriptorPool() {
	VkDescriptorPoolSize typeCount [2];
	typeCount [0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCount [0].descriptorCount = 1;
	if(_useTexture) {
		typeCount [1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		typeCount [1].descriptorCount = 1;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = _useTexture ? 2 : 1;
	descriptorPoolCreateInfo.pPoolSizes = typeCount;

	ErrorCheck(vkCreateDescriptorPool(_renderer->GetVulcanDevice(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool));
}

void Window::deinitDescriptorPool() {
	vkDestroyDescriptorPool(_renderer->GetVulcanDevice(), _descriptorPool, nullptr);
}

void Window::initDescriptorSet() {
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo [1];
	descriptorSetAllocateInfo [0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo [0].pNext = NULL;
	descriptorSetAllocateInfo [0].descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo [0].descriptorSetCount = 1;
	descriptorSetAllocateInfo [0].pSetLayouts = _descriptorSetLayout.data();

	_descriptorSets.resize(1);

	ErrorCheck(vkAllocateDescriptorSets(_renderer->GetVulcanDevice(), descriptorSetAllocateInfo, _descriptorSets.data()));

	VkWriteDescriptorSet writes [2];

	writes [0] = {};
	writes [0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes [0].pNext = NULL;
	writes [0].dstSet = _descriptorSets [0];
	writes [0].descriptorCount = 1;
	writes [0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes [0].pBufferInfo = &_uniformDescriptorInfo;
	writes [0].dstArrayElement = 0;
	writes [0].dstBinding = 0;

	if(_useTexture) {
		writes [1] = {};
		writes [1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes [1].dstSet = _descriptorSets [0];
		writes [1].dstBinding = 1;
		writes [1].descriptorCount = 1;
		writes [1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//writes [1].pImageInfo = &info.texture_data.image_info;
		writes [1].dstArrayElement = 0;
	}

	vkUpdateDescriptorSets(_renderer->GetVulcanDevice(), _useTexture ? 2 : 1, writes, 0, nullptr);

}

void Window::initPipelineCache() {
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	ErrorCheck (vkCreatePipelineCache(_renderer->GetVulcanDevice(), &pipelineCacheCreateInfo, nullptr, &_pipelineCache));
}

void Window::deinitPipelineCache() {
	vkDestroyPipelineCache(_renderer->GetVulcanDevice(), _pipelineCache, nullptr);
}

void Window::initPipeline() {
	VkDynamicState dynamicStateEnables [VK_DYNAMIC_STATE_RANGE_SIZE];

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {};
	
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables;
	pipelineDynamicStateCreateInfo.dynamicStateCount = 0;

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
	
	dynamicStateEnables [pipelineDynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	
	pipelineViewportStateCreateInfo.scissorCount = 1;
	
	dynamicStateEnables [pipelineDynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

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

	ErrorCheck (vkCreateGraphicsPipelines(_renderer->GetVulcanDevice(), _pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &_pipeline));
}

void Window::deinitPipeline() {
	vkDestroyPipeline(_renderer->GetVulcanDevice(), _pipeline, nullptr);
}

Window::~Window() {
	if(_opened) {
		Close();
	}

	if(_win32Instance || _win32Window) {
		_win32Instance = nullptr;
		_win32Window = nullptr;
	}
}

