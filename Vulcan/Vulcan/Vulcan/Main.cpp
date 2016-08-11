#include "VulcanRenderer.h"

int main() {
	VulcanRenderer renderer;

	/*auto device = renderer._deviceHandler;
	VkCommandPool commandPool;
	auto queue = renderer._queue;
	VkFence fence;

	VkFenceCreateInfo fenceCreateInfo {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

	VkSemaphore semaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

	VkCommandPoolCreateInfo commandPoolCreateInfo {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = renderer._graphicFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

	VkCommandBuffer commandBuffer[2];
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 2;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer);

	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(commandBuffer[0], &commandBufferBeginInfo);

		//example of commmand
		VkViewport viewport = {};
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.height = 512;
		viewport.width = 512;
		viewport.x = 0;
		viewport.y = 0;

		vkCmdSetViewport(commandBuffer[0], 0, 1, &viewport);

		vkEndCommandBuffer(commandBuffer[0]);
	}
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(commandBuffer[1], &commandBufferBeginInfo);

		//example of commmand
		VkViewport viewport = {};
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.height = 512;
		viewport.width = 512;
		viewport.x = 0;
		viewport.y = 0;

		vkCmdSetViewport(commandBuffer[1], 0, 1, &viewport);

		vkEndCommandBuffer(commandBuffer[1]);
	}
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[0];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphore;
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[1];

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}
	auto ret = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	//vkQueueWaitIdle(queue);

	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyFence(device, fence, nullptr); */

	renderer.OpenWindow(600, 600, "First Window");
	while(renderer.Run()) {

	}

	return 0;
}
