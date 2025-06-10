#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D diffuse;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexcoord;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(inTexcoord.x,inTexcoord.y,0,1);
  outColor = texture(diffuse, inTexcoord);
}