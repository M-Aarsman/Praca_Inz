#include "VulcanRenderer.h"
#include <cstdlib>
#include <assert.h>

VulcanRenderer::VulcanRenderer() {
	initVulcanInstance();
}


VulcanRenderer::~VulcanRenderer() {
	deinitVulcanInstance();
}

void VulcanRenderer::initVulcanInstance() {
	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	VkResult error = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

	if (error != VK_SUCCESS) {
		assert(0 && "VULCAN ERR: Vulcan create instance error!"); //TODO: add log file and log macro
		std::exit(-1);
	}
}

void VulcanRenderer::deinitVulcanInstance() {
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
}
