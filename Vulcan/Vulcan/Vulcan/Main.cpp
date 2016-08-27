#include "VulcanRenderer.h"
#include "Window.h"

void set_image_layout(VkImage& image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout,
					  VkImageLayout new_image_layout, VulcanRenderer& renderer);

int main() {
	VulcanRenderer renderer; 

	VkSemaphore semaphore = VK_NULL_HANDLE;

	renderer.OpenWindow(600, 600, "First Window");
	Window* window = renderer.GetWindow();
	renderer.ExecuteBeginCommandBuffer();
	renderer.InitDeviceQueue();

	window->InitWindow();

	VkClearValue clear_values [2];
	clear_values [0].color.float32 [0] = 0.2f;
	clear_values [0].color.float32 [1] = 0.2f;
	clear_values [0].color.float32 [2] = 0.2f;
	clear_values [0].color.float32 [3] = 0.2f;
	clear_values [1].depthStencil.depth = 1.0f;
	clear_values [1].depthStencil.stencil = 0;

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo {};
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ErrorCheck (vkCreateSemaphore(renderer.GetVulcanDevice(), &presentCompleteSemaphoreCreateInfo, nullptr, &semaphore));

	uint32_t imageKHRindex = 0;
	ErrorCheck (vkAcquireNextImageKHR(renderer.GetVulcanDevice(), renderer.GetWindow()->GetVulcanSwapchainKHR(), UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageKHRindex));

	set_image_layout(renderer.GetWindow()->GetSwapchainImage()[imageKHRindex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
					 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, renderer);

	VkRenderPassBeginInfo renderPassBeginInfo {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderer.GetWindow()->GetRenderPass();
	renderPassBeginInfo.framebuffer = renderer.GetWindow()->GetFrameBuffer() [imageKHRindex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = renderer.GetWindow()->GetWidth();
	renderPassBeginInfo.renderArea.extent.height = renderer.GetWindow()->GetHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clear_values;

	vkCmdBeginRenderPass(renderer.GetVulcanCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(renderer.GetVulcanCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.GetWindow()->GetPipeline());
	vkCmdBindDescriptorSets(renderer.GetVulcanCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.GetWindow()->GetPipelineLayout(),
							0, 1, renderer.GetWindow()->GetDescriptorSet().data(), 0, nullptr);

	const VkDeviceSize offsets [1] = { 0 };
	vkCmdBindVertexBuffers(renderer.GetVulcanCommandBuffer(), 0, 1, renderer.GetWindow()->GetVertexBuffer(), offsets);

	VkViewport viewport;
	viewport.height = (float) renderer.GetWindow()->GetHeight();
	viewport.width = (float) renderer.GetWindow()->GetWidth();
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	viewport.x = 0;
	viewport.y = 0;

	vkCmdSetViewport(renderer.GetVulcanCommandBuffer(), 0, 1, &viewport);

	VkRect2D rect2d;
	rect2d.extent.width = renderer.GetWindow()->GetWidth();
	rect2d.extent.height = renderer.GetWindow()->GetHeight();
	rect2d.offset.x = 0;
	rect2d.offset.y = 0;
	vkCmdSetScissor(renderer.GetVulcanCommandBuffer(), 0, 1, &rect2d);

	vkCmdDraw(renderer.GetVulcanCommandBuffer(), 12 * 3, 1, 0, 0);
	vkCmdEndRenderPass(renderer.GetVulcanCommandBuffer());

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
	prePresentBarrier.image = renderer.GetWindow()->GetSwapchainImage() [imageKHRindex];

	vkCmdPipelineBarrier(renderer.GetVulcanCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

	ErrorCheck( vkEndCommandBuffer(renderer.GetVulcanCommandBuffer()));

	const VkCommandBuffer cmdBufs [] = { renderer.GetVulcanCommandBuffer() };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;

	ErrorCheck (vkCreateFence(renderer.GetVulcanDevice(), &fenceInfo, nullptr, &drawFence));

	VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submitInfo [1] = {};
	submitInfo [0].pNext = NULL;
	submitInfo [0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo [0].waitSemaphoreCount = 1;
	submitInfo [0].pWaitSemaphores = &semaphore;
	submitInfo [0].pWaitDstStageMask = &pipelineStageFlags;
	submitInfo [0].commandBufferCount = 1;
	submitInfo [0].pCommandBuffers = cmdBufs;
	submitInfo [0].signalSemaphoreCount = 0;
	submitInfo [0].pSignalSemaphores = NULL;

	ErrorCheck( vkQueueSubmit(renderer.GetVulcanQueue(), 1, submitInfo, drawFence));

	VkPresentInfoKHR present {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.swapchainCount = 1;
	present.pSwapchains = renderer.GetWindow()->pGetVulcanSwapchainKHR();
	present.pImageIndices = &imageKHRindex;

	auto res = VK_SUCCESS;
	do {
		res = vkWaitForFences(renderer.GetVulcanDevice(), 1, &drawFence, VK_TRUE, UINT64_MAX);
	} while(res == VK_TIMEOUT);

	assert(res == VK_SUCCESS);
	ErrorCheck( vkQueuePresentKHR(renderer.GetVulcanQueue(), &present));

	Sleep(5000);
	renderer.GetWindow()->Close();

	vkDestroySemaphore(renderer.GetVulcanDevice(), semaphore, nullptr);
	vkDestroyFence(renderer.GetVulcanDevice(), drawFence, nullptr);
	return 0;
}

void set_image_layout(VkImage& image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, 
					  VkImageLayout new_image_layout, VulcanRenderer& renderer){
	
	assert(renderer.GetVulcanCommandBuffer() != VK_NULL_HANDLE);
	assert(renderer.GetVulcanQueue() != VK_NULL_HANDLE);

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

	vkCmdPipelineBarrier(renderer.GetVulcanCommandBuffer(), src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarier);
}

