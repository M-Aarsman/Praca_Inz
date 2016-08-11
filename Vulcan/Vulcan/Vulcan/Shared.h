#pragma once

#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan\vulkan.h>
#include <assert.h>

void ErrorCheck(VkResult reult);