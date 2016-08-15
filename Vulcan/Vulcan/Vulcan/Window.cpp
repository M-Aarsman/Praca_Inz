#include "Window.h"
#include "VulcanRenderer.h"

#include <assert.h>
#include <iostream>

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
	initSwapchain();
	_opened = true;
	
}

void Window::Close() {
	if(_opened) {
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
	if(_surfaceCapabilitesKHR.currentExtent.width < UINT32_MAX) {
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
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

Window::~Window() {
	if(_opened) {
		Close();
	}

	if(_win32Instance || _win32Window) {
		_win32Instance = nullptr;
		_win32Window = nullptr;
	}
}

