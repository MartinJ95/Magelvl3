#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "../samples/utils/geometries.hpp"
#include "../samples/utils/math.hpp"
#include "../samples/utils/shaders.hpp"
#include "../samples/utils/utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"


#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "Vector.h"
#include "RendererSpec.h"


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>
#include <thread>
#include <queue>
#include <stack>
#include <fstream>
#include <optional>

#include <unordered_map>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif



constexpr char AppNameC[] = "01_InitInstance";
constexpr char EngineNameC[] = "Vulkan.hpp";

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

constexpr char VertexShader[] = R"(
#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform buffer
{
	mat4 mvp;
} uniformBuffer;

layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 InTexCoord;

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outTexCoord;

void main()
{
	outColor = inColor;
	outPos = uniformBuffer.mvp * vec4(pos.xyz, 1.0f);
	outTexCoord = InTexCoord;
	gl_Position = outPos;
}
)";

constexpr char FragmentShader[] = R"(
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
}
)";

const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


static std::string AppName = "01_InitInstance";
static std::string EngineName = "Vulkan.hpp";

extern void (*InputFunction)(int, int, int, int);

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	InputFunction(key, scancode, action, mods);
}

struct QueueFamilyIndices {
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;
	bool IsComplete() {
		return GraphicsFamily.has_value() && PresentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct Vertex
{
	Vertex(glm::vec4 Position, glm::vec4 Color, glm::vec2 TexCoord, int MaterialID = 0) : m_position(Position), m_color(Color), m_uv(TexCoord), m_materialID(MaterialID) {}
	glm::vec4 m_position;
	glm::vec4 m_color;
	glm::vec2 m_uv;
	int m_materialID;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, m_position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, m_color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, m_uv);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
		attributeDescriptions[3].offset = offsetof(Vertex, m_materialID);

		return attributeDescriptions;
	}
};

struct TextureData
{
	VkImage m_diffuseTexture;
	VkDeviceMemory textureImageMemory;
};

struct ShadowUniformBufferObject
{
	glm::mat4 m_shadowModelViewProj;
};

struct UniformBufferObject
{
	glm::mat4 m_veiwMatrix;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_lightViewMatrix;
};

constexpr int gImageBufferAmount = 6;

struct UniformTextureBuffer
{
	std::array<VkSampler, gImageBufferAmount> m_diffuseTextures;
	VkSampler m_shadowMap;
	//glm::mat4 m_transform;
};

struct PushConstant
{
	glm::mat4 m_transform;
	uint32_t m_usesTexture;
};

static std::vector<Vertex> GenerateSphere()
{
	std::vector<Vertex> Verts;

	glm::vec3 Up{ 0,1,0 };
	glm::vec3 Forward{ 1,0,0 };
	glm::vec3 Side{ 0,0,1 };

	for (int i = 0; i < 2; i++)
	{
		glm::vec3 offset = Forward * 10.f * (i*2.f-1.f);
		for (int j = 0; j < 50; j++)
		{
			for (int k = 0; k < 50; k++)
			{
				auto GetBasePosOffset = [](glm::vec4 Offset, int Index, float Size) -> glm::vec4
					{
						return Offset * (Index - Size * 0.5f);
					};
				
				glm::vec4 Offset = glm::vec4(offset, 1);
				glm::vec4 secondOffset = glm::vec4(Up, 1);
				glm::vec4 thirdOffset = glm::vec4(Side, 1);


				glm::vec4 basePos = Offset + GetBasePosOffset(secondOffset, j, 10.f) + GetBasePosOffset(thirdOffset, k, 10.f);

				glm::vec4 col = glm::vec4(1, 0, 0, 1);

				auto ApplyWrapAround = [&](glm::vec4& Pos)
					{
						float d = glm::abs(glm::dot(Side, glm::vec3(Pos.x, Pos.y, Pos.z)));
						d = std::max(glm::abs(glm::dot(Up, glm::vec3(Pos.x, Pos.y, Pos.z))), d);

						Pos += (Offset * glm::vec4(-1, -1, -1, 1)) * (d*0.2f);
						//Pos *= 0.1f;
						
						Pos = glm::vec4(glm::normalize(glm::vec3(Pos.x, Pos.y, Pos.z)),1);
					};

				Verts.emplace_back(basePos, col, glm::vec2(j / 50.f, (k + 1) / 50.f));
				ApplyWrapAround(Verts.back().m_position);
				Verts.emplace_back((basePos + secondOffset), col, glm::vec2((j + 1) / 50.f, (k + 1) / 50.f));
				ApplyWrapAround(Verts.back().m_position);
				Verts.emplace_back((basePos + thirdOffset), col, glm::vec2(j / 50.f, k / 50.f));
				ApplyWrapAround(Verts.back().m_position);

				Verts.emplace_back((basePos + secondOffset), col, glm::vec2(j / 50.f, k / 50.f));
				ApplyWrapAround(Verts.back().m_position);
				Verts.emplace_back((basePos + secondOffset + thirdOffset), col, glm::vec2((j + 1) / 50.f, (k + 1) / 50.f));
				ApplyWrapAround(Verts.back().m_position);
				Verts.emplace_back((basePos + thirdOffset), col, glm::vec2((j + 1) / 50.f, k / 50.f));
				ApplyWrapAround(Verts.back().m_position);
			}
		}
	}

	return Verts;
}

static std::vector<Vertex> GenerateBox()
{
	std::vector<Vertex> Verts;

	for (int i = 0; i < 6; i++)
	{
		int NormalDimension = 0;
		if (i < 2)
		{
			NormalDimension = 0;
		}
		else if (i < 4)
		{
			NormalDimension = 1;
		}
		else
		{
			NormalDimension = 2;
		}
		int OtherDimensions[2]{ -1, -1 };
		{
			int CurrentTest = 0;
			int CurrentDimension = 0;

			while (OtherDimensions[1] == -1)
			{
				if (CurrentTest != NormalDimension)
				{
					OtherDimensions[CurrentDimension] = CurrentTest;
					CurrentDimension++;
				}
				CurrentTest++;
			}
		}

		glm::vec4 offset = glm::vec4(10, 10, 10, 1);
		
		//each dimension is either 1 or -1
		offset *= glm::vec4(-2 * (i%2)+1, -2 * (i%2)+1, -2 * (i%2)+1, 1);

		//halfs the dimension
		offset *= 0.5f;

		glm::vec4 secondOffset = glm::vec4(0, 0, 0, 1);
		secondOffset[OtherDimensions[0]] = 1;
				
		glm::vec4 thirdOffset = glm::vec4(0, 0, 0, 1);
		thirdOffset[OtherDimensions[1]] = 1;

		for (auto& Dimension : OtherDimensions)
		{
			offset[Dimension] = 0;
		}

		for (int j = 0; j < 10; j++)
		{
			for (int k = 0; k < 10; k++)
			{
			
				auto GetBasePosOffset = [](glm::vec4 Offset, int Index, float Size) -> glm::vec4
					{
						return Offset * (Index - Size * 0.5f);
					};

				glm::vec4 basePos = offset + GetBasePosOffset(secondOffset, j, 10.f) + GetBasePosOffset(thirdOffset, k, 10.f);

				glm::vec4 col = glm::vec4(1, 0, 0, 1);

				constexpr float scaleAmount = 0.1f;

				//called when value is 1
				auto MaxOffset = [](float Pos) -> float {
					return 1.f * ((Pos+1) * 0.1f);
					};
				auto MinOffset = [](float Pos) -> float {
					return 0.f + (0.1f * Pos);
					};

				if ((i % 2 == 0 && i != 2) || i == 3)
				{

					Verts.emplace_back(basePos* scaleAmount, col, glm::vec2(MinOffset(k), MinOffset(j)));
					Verts.emplace_back((basePos + secondOffset)* scaleAmount, col, glm::vec2(MinOffset(k), MaxOffset(j)));
					Verts.emplace_back((basePos + thirdOffset)* scaleAmount, col, glm::vec2(MaxOffset(k), MinOffset(j)));

					Verts.emplace_back((basePos + secondOffset)* scaleAmount, col, glm::vec2(MinOffset(k), MaxOffset(j)));
					Verts.emplace_back((basePos + secondOffset + thirdOffset)* scaleAmount, col, glm::vec2(MaxOffset(k), MaxOffset(j)));
					Verts.emplace_back((basePos + thirdOffset)* scaleAmount, col, glm::vec2(MaxOffset(k), MinOffset(j)));
					continue;
				}
				Verts.emplace_back(basePos * scaleAmount, col, glm::vec2(MinOffset(k), MinOffset(j)));
				Verts.emplace_back((basePos + thirdOffset) * scaleAmount, col, glm::vec2(MaxOffset(k), MinOffset(j)));
				Verts.emplace_back((basePos + secondOffset) * scaleAmount, col, glm::vec2(MinOffset(k), MaxOffset(j)));

				Verts.emplace_back((basePos + secondOffset) * scaleAmount, col, glm::vec2(MinOffset(k), MaxOffset(j)));
				Verts.emplace_back((basePos + thirdOffset) * scaleAmount, col, glm::vec2(MaxOffset(k), MinOffset(j)));
				Verts.emplace_back((basePos + secondOffset + thirdOffset) * scaleAmount, col, glm::vec2(MaxOffset(k), MaxOffset(j)));
			}
		}
	}

	return Verts;
}

struct GraphicsModel
{
	GraphicsModel(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device,  const std::vector<Vertex>&& Vertices, const std::vector<uint16_t>&& Elements) :
		m_vertices(Vertices), m_elements(Elements)/*, m_vertexBufferData(PhysicalDevice, Device, sizeof(Vertex)* m_vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer)*/
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(m_vertices[0]) * m_vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, m_vertexBuffer, &memRequirements);

		auto findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type");

			};

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(Device, &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(Device, m_vertexBuffer, m_vertexBufferMemory, 0);

		void* data;
		vkMapMemory(Device, m_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, m_vertices.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(Device, m_vertexBufferMemory);

		//vk::su::copyToDevice(Device, m_vertexBufferData.deviceMemory, (VertexPC*)&m_vertices[0], m_vertices.size() /*sizeof(coloredCubeData) / sizeof(coloredCubeData[0])*/);
	}
	GraphicsModel(const GraphicsModel& other) = delete;
	GraphicsModel(GraphicsModel&& other) = delete;
	void operator = (const GraphicsModel & other) = delete;
	void operator=(GraphicsModel&& other) = delete;

	void InitIndices(VkPhysicalDevice& PhysicalDevice, VkDevice& Device)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(m_elements[0]) * m_elements.size();
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Device, &bufferInfo, nullptr, &m_indexBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create index buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, m_indexBuffer, &memRequirements);
		auto findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type");

			};

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(Device, &allocInfo, nullptr, &m_indexBufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(Device, m_indexBuffer, m_indexBufferMemory, 0);

		void* data;
		vkMapMemory(Device, m_indexBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, m_elements.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(Device, m_indexBufferMemory);
	}

	unsigned int m_renderingPassId;
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_elements;
	//vk::su::BufferData m_vertexBufferData;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};

/*
* Graphics Object
* 
* holds multiple sub meshes when loading an obj with multiple shapes
*/
struct GraphicsObject
{
	GraphicsObject() :
		SubModels()
	{}
	std::unordered_map<unsigned int, GraphicsModel> SubModels;
};

struct TerrainSubIndices
{
	TerrainSubIndices() :
		m_elements(),
		m_indexBuffer(),
		m_indexMemory()
	{};
	TerrainSubIndices(const TerrainSubIndices& Other) :
		m_elements(Other.m_elements),
		m_indexBuffer(Other.m_indexBuffer),
		m_indexMemory(Other.m_indexMemory)
	{}
	TerrainSubIndices(TerrainSubIndices&& Other) noexcept :
		m_elements(Other.m_elements),
		m_indexBuffer(std::move(Other.m_indexBuffer)),
		m_indexMemory(std::move(Other.m_indexMemory))
	{}
	void operator=(const TerrainSubIndices& Other)
	{
		m_elements = Other.m_elements;
		m_indexBuffer = Other.m_indexBuffer;
		m_indexMemory = Other.m_indexMemory;
	}
	void operator=(TerrainSubIndices&& Other) noexcept
	{
		m_elements = Other.m_elements;
		m_indexBuffer = std::move(Other.m_indexBuffer);
		m_indexMemory = std::move(Other.m_indexMemory);
	}
	std::vector<uint16_t> m_elements;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexMemory;
	void InitIndices(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(m_elements[0]) * m_elements.size();
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Device, &bufferInfo, nullptr, &m_indexBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create index buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, m_indexBuffer, &memRequirements);
		auto findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type");

			};

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(Device, &allocInfo, nullptr, &m_indexMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(Device, m_indexBuffer, m_indexMemory, 0);

		void* data;
		vkMapMemory(Device, m_indexMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, m_elements.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(Device, m_indexMemory);
	}
};

struct Terrain
{
	Terrain(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const int Size) :
		m_sectionIndices(),
		m_TerrainModel(
			PhysicalDevice,
			Device,
			GenVertices(Size),
			GenElements(PhysicalDevice, Device, Size)
		),
		m_size(Size)
	{}
	std::vector<TerrainSubIndices> m_sectionIndices;
	std::vector<glm::vec3> m_treePositions;
	GraphicsModel m_TerrainModel;
	int m_size;
	std::vector<Vertex> GenVertices(const int Size)
	{
		std::vector<Vertex> verts;
		verts.reserve(Size * Size);

		for (int i = 0; i < Size; i++)
		{
			for (int j = 0; j < Size; j++)
			{
				verts.emplace_back(glm::vec4(i - Size * 0.5f, -5, j - Size *0.5, 1), glm::vec4(0, 1, 0, 1), glm::vec2((i*i)%2, (j*j)%2));
				verts.emplace_back(glm::vec4((i + 1) - Size * 0.5f, -5, j - Size*0.5, 1), glm::vec4(0, 1, 0, 1), glm::vec2(((i*i)+1)%2, (j*j)%2));

				if (i == 0)
				{
					verts[verts.size()-2].m_position.y += ((double)rand() / (RAND_MAX));
				}
				else
				{
					verts[verts.size() - 2].m_position.y = verts[verts.size()-((Size*2)+1)].m_position.y;
				}
				verts.back().m_position.y += ((double)rand() / (RAND_MAX));
			}
		}
		
		for (int i = 0; i < 100; i++)
		{
			m_treePositions.emplace_back(glm::vec3(
				(float)(rand() % Size) - Size*0.5f,
				-2.f,
				(float)(rand() % Size) - Size * 0.5f));
		}
		
		

		return verts;
	}
	std::vector<uint16_t> GenElements(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const int Size)
	{
		std::vector<uint16_t> elements;

		m_sectionIndices.reserve(Size);

		for (int i = 0; i <= Size; i++)
		{
			m_sectionIndices.emplace_back();
			for (int j = 0; j < Size*2; j=j+2)
			{
				m_sectionIndices.back().m_elements.emplace_back(j+1 + (Size*2) * i);
				m_sectionIndices.back().m_elements.emplace_back(j + (Size*2) * i);
			}
			m_sectionIndices.back().InitIndices(PhysicalDevice, Device);
		}

		return elements;
	}
};

struct Mesh
{
	bool LoadFromObj(const char* filename, std::vector<std::vector<Vertex>>& verts, std::vector<std::vector<uint16_t>>& elements);
};

constexpr unsigned int ModelBufferAmount = 5*5*5+10;

static std::vector<char> ReadFile(const std::string& Filename)
{
	std::ifstream file(Filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

struct GraphicsRenderPassOptions
{
	std::string VertexShader;
	std::string FragmentShader;
	VkPrimitiveTopology Topology;
	VkAttachmentLoadOp LoadOp;
	VkAttachmentStoreOp StoreOp;
	bool IsShadowMapper;
};

constexpr float gNearPlane = 0.1f;
constexpr float gFarPlane = 1000.f;

static glm::mat4 GetViewProjectionMatrix(const VkExtent2D& SurfaceExtent, const glm::mat4& CamMatrix)
{
	//glm::mat4x4 testMat = vk::su::createModelViewProjectionClipMatrix(SurfaceExtent);

	glm::mat4x4 viewMatrix = glm::inverse(CamMatrix);

	glm::mat4x4 xMatrix = glm::inverse(
		glm::mat4x4(
			1, 0, 0, 0,
			0, -1, 0, 0,
			0, 0, -1, 0,
			0, 0, 0, 1
		)
	);

	float aspectRatio = SurfaceExtent.width / SurfaceExtent.height;

	float fov = 45;

	std::pair<float, float> nearfarplanes{ 0.1f,100.f };

	glm::mat4x4 projectionMatrix = glm::perspective(fov, aspectRatio, nearfarplanes.first, nearfarplanes.second);
	/*
	glm::mat4x4 projectionMatrix = glm::mat4x4(
		aspectRatio/tan(fov*0.5), 0, 0, 0,
		0, 1/tan(fov*0.5), 0, 0,
		0, 0, nearfarplanes.second/ nearfarplanes.first - nearfarplanes.second, -(nearfarplanes.first*nearfarplanes.second / nearfarplanes.second-nearfarplanes.first),
		0, 0, 1, 0
	);
	*/

	glm::mat4 projectionViewMatrix = projectionMatrix * xMatrix * viewMatrix;

	return projectionViewMatrix;
}

static glm::mat4 GetProjectionMatrix(const VkExtent2D& SurfaceExtent)
{
	float aspectRatio = SurfaceExtent.width / SurfaceExtent.height;

	float fov = 45.f;

	std::pair<float, float> nearFarPlanes{ gNearPlane, gFarPlane };

	glm::mat4 projectionMatrix = glm::perspective(fov, aspectRatio, nearFarPlanes.first, nearFarPlanes.second);

	return projectionMatrix;
}
static glm::mat4 GetViewMatrix(const glm::mat4& CamMatrix)
{

	glm::mat4 viewMatrix = glm::inverse(CamMatrix);

	glm::mat4x4 xMatrix = glm::inverse(
		glm::mat4x4(
			1, 0, 0, 0,
			0, -1, 0, 0,
			0, 0, -1, 0,
			0, 0, 0, 1
		)
	);


	return xMatrix * viewMatrix;
}

static VkShaderModule NewCreateShaderModule(const VkDevice& Device, const std::vector<char>& Code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = Code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

struct GraphicsShadowPass
{
public:
	void Init(const VkPhysicalDevice PhysicalDevice, const VkDevice Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const VkImageView& ShadowDepthImageView, const VkSurfaceKHR& Surface, const GraphicsRenderPassOptions Options);
	void CreateRenderPass(const VkPhysicalDevice PhysicalDevice, const VkDevice& Device, const VkFormat& SwapChainImageFormat, const VkAttachmentLoadOp& LoadOp, const VkAttachmentStoreOp StoreOp);
	void CreateDescriptorSetLayout(const VkDevice& Device);
	void CreateDescriptorSet(const VkDevice& Device);
	void CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device);
	void CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent, const std::string& VertexShader, const std::string& FragmentShader, const VkPrimitiveTopology& Topology);
	void CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const VkImageView& ShadowDepthImageView, const VkExtent2D& SwapChainExtent);
	void CreateCommandPool(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface);
	void CreateCommandBuffer(const VkDevice& Device);
	void NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const ShadowUniformBufferObject BufferObject, const std::array<VkSampler, gImageBufferAmount>& Textures, const std::array<VkImageView, gImageBufferAmount>& ImageViews, const VkImageView& ShadowMapImageView);
	void NewOnRendorObjBegin(const VkBuffer& VertexBuffer, const VkBuffer& IndexBuffer);
	void NewOnRendorObj(const int VertAmount, const PushConstant& Uniform, const int IndexAmount);
	void NewOnRenderFinish();
public:
	VkRenderPass m_renderPass;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_uniformDescriptorSet;
	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformBufferMemory;
	std::vector<void*> m_uniformBuffersMapped;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
	VkPipelineLayout m_pipelineLayout;
	VkPipelineCache m_pipelineCache;
	VkPipeline m_pipeline;
	std::vector<VkFramebuffer> m_frameBuffers;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
};

struct GraphicsRenderPass
{
	GraphicsRenderPass() :
		m_modelID(),
		m_mvpcMatrix(),
		m_renderPass(),
		m_vertexShaderModule(),
		m_fragmentShaderModule(),
		m_frameBuffers(),
		m_pipelineLayout(),
		m_descriptorSetLayout(),
		m_descriptorPool(),
		m_commandPool(),
		m_commandBuffer(),
		m_usedModelsAmount(0),
		m_descriptorSets(),
		m_pipelineCache(),
		m_pipeline(),
		imageAcquiredSemaphore(),
		m_isShadowMapping(false)
	{}
	void Init(const VkPhysicalDevice& PhysicalDevice, const VkDevice &Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const std::vector<VkImageView>& ShadowMapImageView, const VkImageView& ShadowDepthImageView, const VkImageView DepthImageView, const VkSurfaceKHR& Surface, const GraphicsRenderPassOptions Options);
	void CreateRenderPass(const VkPhysicalDevice PhysicalDevice, const VkDevice& Device, const VkFormat& SwapChainImageFormat, const VkAttachmentLoadOp& LoadOp, const VkAttachmentStoreOp& StoreOp);
	void CreateDescriptorSetLayout(const VkDevice& Device);
	void CreateComputeDescriptorSetLayout(const VkDevice& Device);
	void CreateDescriptorSet(const VkDevice& Device);
	void CreateComputeDescriptorSet(const VkDevice& Device);
	void CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device);
	void CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent, const std::string& VertexShader, const std::string& FragmentShader, const VkPrimitiveTopology& Topology);
	void CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const std::vector<VkImageView>& ShadowMapImageView, const VkImageView& ShadowDepthImageView, const VkImageView DepthImageView, const VkExtent2D& SwapChainExtent);
	void CreateCommandPool(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface);
	void CreateCommandBuffer(const VkDevice& Device);
	void RecordCommandBuffer(const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent);
	VkShaderModule CreateShaderModule(const VkDevice& Device, const std::vector<char>& Code);
	void NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const UniformBufferObject BufferObject, const std::array<VkSampler, gImageBufferAmount>& Textures, const std::array<VkImageView, gImageBufferAmount>& ImageViews, const VkImageView& ShadowMapImageView);
	void NewOnRendorObjBegin(const VkBuffer& VertexBuffer, const VkBuffer& IndexBuffer);
	void NewOnRendorObj(const int VertAmount, const PushConstant& Uniform, const int IndexAmount);
	void NewOnRenderFinish();
	void CleanUp(const vk::Device& Device);
	~GraphicsRenderPass();
public:
	std::vector<unsigned int> m_modelID;
	glm::mat4x4 m_mvpcMatrix;
	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformBufferMemory;
	std::vector<void*> m_uniformBuffersMapped;
	VkRenderPass m_renderPass;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
	std::vector<VkFramebuffer> m_frameBuffers;
	VkPipelineLayout m_pipelineLayout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	uint32_t m_descriptorPoolSize;
	VkDescriptorSet m_uniformDescriptorSet;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	unsigned int m_usedModelsAmount{0};
	std::vector<vk::DescriptorSet> m_descriptorSets;
	VkPipelineCache m_pipelineCache;
	VkPipeline m_pipeline;
	vk::Semaphore imageAcquiredSemaphore;
	bool m_isShadowMapping;
};
/*
struct GuiRenderPass
{
	GuiRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const vk::su::SurfaceData& SurfaceData, std::pair<uint32_t, uint32_t>& GraphicsAndPresentQueueFamilyIndex, const vk::su::SwapChainData& SwapChainData);
	vk::su::DepthBufferData m_depthBufferData;
	VkDescriptorPool m_descriptorPool;
	VkRenderPass m_renderPass;
	VkCommandBuffer m_commandBuffer;
	std::vector<VkFramebuffer> m_frameBuffers;
};
*/

/*
* Sub Queue for render targets
* contains mapped sub objects and the corresponding transforms where drawn
* note that we treat unsigned int as a bit array so allow graphics objects to have 32 sub shapes max
*/
struct RenderObjectSubQueue
{
	std::unordered_map<unsigned int, std::queue<glm::mat4>> m_subQueue;
};
/*
* SubQueue
* 
*/
struct RenderObjectQueue
{
	std::unordered_map<unsigned int, RenderObjectSubQueue> m_mainObjectRenderQueue;
};

class Renderer : public RendererSpec
{
public:
	Renderer(int Width, int Height);
	void Init() override final;
	void CreateSyncObjects();
	void CreateSingleUseCommandPoolAndBuffer();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateSurface();
	void CreateImageViews();
	void CreateShadowMapImageViews();
	void CreateViewportImageVies();
	void CreateGraphicsPipeline();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT MessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData,
		void* PUserData);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice& Device);
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);
	std::vector<const char*> GetRequiredExtensions();
	void SetupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* Allocator);
	int RateDeviceSuitability(VkPhysicalDevice Device);
	bool IsDeviceSuitable(VkPhysicalDevice Device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);
	bool CheckValidationLayerSupport();
	static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& Device, const VkSurfaceKHR& Surface);
	void PickPhysicalDevice();
	void AddToRenderQueue(const unsigned int RenderPass, const Vector3& Pos, const unsigned int ModelID) override final;
	glm::mat4 BuildMatrix(const Vector3& Position, const Vector3& Rotation);
	void PositionCamera(const Vector3& Position, const Vector3& Rotation) override final;
	void PositionLight(const Vector3& Position, const Vector3& Rotation);
	void Render(const float DeltaTime) override final;
	bool WindowShouldClose() const override final;
	void PollEvents() override final;
	void OnGUI();
	static uint32_t FindMemoryType(const VkPhysicalDevice& PhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void CreateBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	bool HasStencilComponent(VkFormat Format);
	static VkFormat FindSupportedFormat(const VkPhysicalDevice PhysicalDevice, const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);
	static VkFormat FindDepthFormat(const VkPhysicalDevice PhysicalDevice);
	void CreateDepthResources();
	void CreateTextureImage();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer CommandBuffer);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
	void CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height);
	void CreateTextureImageView();
	VkImageView CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags);
	void CreateTextureSampler();
	void CreateInstance();
	~Renderer();
public:
	int m_width, m_height;
	VkInstance m_vulkanInstance;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger;
	GLFWwindow* m_window;
	VkExtent2D m_extents;
	VkSurfaceKHR m_surface;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::pair<uint32_t, uint32_t> m_graphicsAndPresentQueueFamilyIndex;
	VkDevice m_device;
	VkCommandPool m_pool;
	VkCommandBuffer m_commandBuffer;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkSwapchainKHR m_swapChain;

	std::vector<VkImage> m_viewportImages;
	std::vector<VkImageView> m_viewportImageViews;
	std::vector<VkDeviceMemory> m_viewportImageMemory;

	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;

	std::vector<VkImage> m_shadowMapImages;
	std::vector<VkDeviceMemory> m_shadowMapImagesMemory;
	std::vector<VkImageView> m_shadowMapImageViews;

	VkDescriptorSet m_viewportDescriptorSet;

	VkImage m_shadowMapDepthImage;
	VkDeviceMemory m_shadowMapDepthMemory;
	VkImageView m_shadowMapDepthImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::unordered_map<unsigned int, Terrain> m_terrain;
	std::unordered_map<unsigned int, GraphicsObject> m_modelDatas;
	/*
	* RenderTargets
	* Key = render pass
	* Value = mapped pairs
	* key = graphics model
	* value = transform matrix
	*/
	std::unordered_map<unsigned int, RenderObjectQueue> m_shadowTargets;
	std::unordered_map<unsigned int, RenderObjectQueue> m_renderingTargets;
	std::unordered_map<unsigned int, GraphicsRenderPass> m_renderPasses;
	std::unordered_map<unsigned int, GraphicsShadowPass> m_shadowPasses;
	GraphicsRenderPass m_guiPass;
	std::unordered_map<unsigned int, VkImage> m_imageDatas;
	std::unordered_map<unsigned int, VkDeviceMemory> m_imageMemory;
	std::array<VkImageView, gImageBufferAmount> m_imageVeiws;
	std::array<VkSampler, gImageBufferAmount> m_samplers;
	//GuiRenderPass m_guiPass;
	glm::mat4x4 m_camMatrix = glm::mat4x4(1);
	glm::mat4 m_lightMatrix = glm::mat4(1);
	ImGui_ImplVulkanH_Window mainWindowData;
	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;
	VkFence m_inFlightFence;
	std::pair<float, float> m_upDateDescriptorTimer{ 0, 1.f };
	glm::vec3 m_directionalLightDirection{ 1, -0.5, 0 };
	bool m_shouldUpdateDescriptor{ true };
};