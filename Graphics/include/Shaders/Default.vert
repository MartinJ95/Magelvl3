#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
	mat4 lightView;
} ubo;

layout(push_constant) uniform Push {
	mat4 mvp;
	bool usesTexture;
} push;

layout(binding = 1) uniform sampler2D diffuse;
layout(binding = 2) uniform sampler2D shadowMap;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 InTexCoord;
layout(location = 3) in int inTextureIndex;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) flat out int outTextureIndex;
layout(location = 4) out vec4 inUnmoddedPosition;

void main()
{
	mat4 view = ubo.view;
	outPos = vec3(push.mvp * vec4(pos.xyz, 1.0f));
	//outPos = push.mvp * vec4(pos.xyz, 1.0f);
	outColor = inColor;
	outTexCoord = InTexCoord;
	outTextureIndex = inTextureIndex;
	inUnmoddedPosition = ubo.proj * ubo.lightView * vec4(outPos, 1);
	gl_Position = ubo.proj * view * push.mvp * vec4(pos.xyz, 1);
}