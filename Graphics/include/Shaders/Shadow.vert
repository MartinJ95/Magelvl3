#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 LightViewProjection;
} ubo;

layout(push_constant) uniform Push {
	mat4 mvp;
	bool usesTexture;
} push;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 InTexCoord;
layout(location = 3) in int inTextureIndex;

void main()
{
	
	gl_Position = ubo.LightViewProjection * push.mvp * vec4(pos.xyz, 1);
}