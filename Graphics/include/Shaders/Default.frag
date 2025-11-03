#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
	mat4 lightView;
} ubo;

layout(push_constant) uniform Push {
	mat4 mvp;
	bool usesTexture;
} push;

layout (binding = 1) uniform sampler2D diffuse[];
layout (binding = 2) uniform sampler2D shadowMap;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexcoord;
layout(location = 3) flat in int inTextureIndex;

layout(location = 4) in vec4 inUnmoddedPosition;

layout (location = 0) out vec4 outColor;

void main()
{

//vec4 position =  ubo.proj * ubo.lightView * vec4(inUnmoddedPosition.xyz, 1);
vec4 position =  inUnmoddedPosition;

vec3 projCoords = position.xyz/position.w;
//position.xyz = position.xyz/position.w;
projCoords.xy = projCoords.xy * 0.5 + 0.5;
  if(!push.usesTexture)
  {
    outColor = inColor;
  }
else
  {
    outColor = vec4(inTexcoord.x,inTexcoord.y,0,1);
    outColor = texture(diffuse[inTextureIndex], inTexcoord);
  }

  if(texture(shadowMap, projCoords.xy).r < projCoords.z - 0.00005)
  {
    outColor.x = 0;//min(outColor.x, 0.2);
    outColor.y = 0;//min(outColor.y, 0.2);
    outColor.z = 0;//min(outColor.z, 0.2);
  }
//outColor.w = 1;
//outColor.x = texture(shadowMap, projCoords.xy).x;
//outColor.y = texture(shadowMap, projCoords.xy).y;
//outColor.z = texture(shadowMap, projCoords.xy).z;
//outColor.x = projCoords.x;
//outColor.y = projCoords.y;
//outColor.z = projCoords.z;
//outColor.y = position.y;
//outColor.z = position.z;
//outColor = vec4(texture(shadowMap, vec2(position.xy)).z - position.z/position.w, 0, 0, 1);
}