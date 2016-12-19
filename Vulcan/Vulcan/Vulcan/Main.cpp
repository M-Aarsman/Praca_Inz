#include "Window.h"

#include "VulkanRenderer.h"

int main() {
	VulkanRenderer renderer(18000); 
	VkSemaphore semaphore = VK_NULL_HANDLE;
	while(renderer.Run()) {
	}

	renderer.DeinitVertexBuffer();
	renderer.GetWindow()->Close();
	ErrorCheck(vkEndCommandBuffer(renderer.GetVulcanCommandBuffer()));
	
	return 0;
}
