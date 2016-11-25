#pragma once

#define VK_USE_PLATFORM_WIN32_KHR 1

#include <vulkan\vulkan.h>
#include <assert.h>
#include <SPIRV\GlslangToSpv.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

enum Key {
	KEY_UP,
	KEY_DOWN,
	KEY_NUM
};

struct Vertex {
	glm::vec4 position; // Position data
	glm::vec4 color;
};

struct VertexUV {
	float u, v;                   // texture u,v
	glm::vec4 position;
};

void ErrorCheck(VkResult reult);

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);
void init_resources(TBuiltInResource &Resources);

bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *shaderText,
			   std::vector<unsigned int> &spirv);