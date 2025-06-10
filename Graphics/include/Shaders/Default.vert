#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform Push {
	mat4 mvp;
} push;

layout(binding = 0) uniform sampler2D diffuse;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 InTexCoord;

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outTexCoord;

void main()
{
	outPos = push.mvp * vec4(pos.xyz, 1.0f);
	outColor = inColor;
	outTexCoord = InTexCoord;
	gl_Position = outPos;
}