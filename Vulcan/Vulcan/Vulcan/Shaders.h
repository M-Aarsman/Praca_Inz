#pragma once
static const char *vertShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable				\n"
"#extension GL_ARB_shading_language_420pack : enable			\n"
"																\n"
"layout (std140, binding = 0) uniform bufferVals {				\n"
"    mat4 mvp;													\n"
"} myBufferVals;												\n"
"																\n"
"layout (location = 0) in vec4 pos;								\n"
"layout (location = 1) in vec4 inColor;							\n"
"layout (location = 0) out vec4 outColor;						\n"
"																\n"
"out gl_PerVertex {												\n"
"    vec4 gl_Position;											\n"
"};																\n"
"																\n"
"void main() {													\n"
"   outColor = inColor;											\n"
"   gl_Position = myBufferVals.mvp * pos;						\n"
"}																\n";

static const char *fragShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (location = 0) in vec4 color;\n"
"layout (location = 0) out vec4 outColor;\n"
"void main() {\n"
"   outColor = color;\n"
"}\n";