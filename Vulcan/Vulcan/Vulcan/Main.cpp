#include "Window.h"

#include "VulkanRenderer.h"
#include "VulkanWindow.h"

int main() {
	VulkanRenderer renderer(500); 
	VkSemaphore semaphore = VK_NULL_HANDLE;

	renderer.OpenWindow(600, 600, "First Window");
	renderer.ExecuteBeginCommandBuffer();
	renderer.InitDeviceQueue();

	Window* window = renderer.GetWindow();
	window->InitWindow();
	renderer.InitVertexBuffer();
	
	uint32_t imageKHRindex = 0;


	while(renderer.Run(imageKHRindex)) {
	}

	renderer.DeinitVertexBuffer();
	renderer.GetWindow()->Close();
	ErrorCheck(vkEndCommandBuffer(renderer.GetVulcanCommandBuffer()));
	
	return 0;
}
