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


#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "Vector.h"
#include "RendererSpec.h"


#include "imgui.h"
#include "imgui-1.91.8/backends/imgui_impl_glfw.h"
#include "imgui-1.91.8/backends/imgui_impl_vulkan.h"

#include <iostream>
#include <thread>
#include <queue>
#include <stack>
#include <fstream>
#include <optional>
#include <tiny_obj_loader.h>

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
	Vertex(glm::vec4 Position, glm::vec4 Color, glm::vec2 TexCoord) : m_position(Position), m_color(Color), m_uv(TexCoord) {}
	glm::vec4 m_position;
	glm::vec4 m_color;
	glm::vec2 m_uv;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

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


		return attributeDescriptions;
	}
};

struct TextureData
{
	VkImage m_diffuseTexture;
	VkDeviceMemory textureImageMemory;
};

struct UniformBuffer
{
	glm::mat4 m_Transfrom;
};

struct PushConstant
{
	glm::mat4 m_transform;
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
	GraphicsModel(VkPhysicalDevice& PhysicalDevice, VkDevice& Device,  std::vector<Vertex>&& Vertices, std::vector<unsigned int>&& Elements) :
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

		auto& findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
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
			throw std::runtime_error("failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device, m_indexBuffer, &memRequirements);

		auto& findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
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
		memcpy(data, m_vertices.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(Device, m_indexBufferMemory);
	}

	unsigned int m_renderingPassId;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_elements;
	//vk::su::BufferData m_vertexBufferData;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
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
		imageAcquiredSemaphore()
	{}
	void Init(const VkPhysicalDevice& PhysicalDevice, const VkDevice &Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const VkSurfaceKHR& Surface);
	void CreateRenderPass(const VkDevice& Device, const VkFormat& SwapChainImageFormat);
	void CreateDescriptorSetLayout(const VkDevice& Device);
	void CreateDescriptorSet(const VkDevice& Device);
	void CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device);
	void CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent);
	void CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const VkExtent2D& SwapChainExtent);
	void CreateCommandPool(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface);
	void CreateCommandBuffer(const VkDevice& Device);
	void RecordCommandBuffer(const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent);
	VkShaderModule CreateShaderModule(const VkDevice& Device, const std::vector<char>& Code);
	glm::mat4 GetViewProjectionMatrix(const VkExtent2D& SurfaceExtent, const glm::mat4& CamMatrix);
	void SetUniformDataModelViewProjection(const glm::mat4& projectionViewMatrix,const vk::su::SurfaceData& SurfaceData, const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const glm::mat4x4& ModelMatrix, const glm::mat4x4& CamMatrix, const bool ShouldUpdate);
	void OnRenderStart(const vk::Device& Device, vk::su::SwapChainData& SwapChainData, vk::CommandBuffer& CommandBuffer, vk::su::SurfaceData& SurfaceData);
	void OnRenderObj(const vk::CommandBuffer& CommandBuffer, const VkBuffer& VertexBuffer, const std::vector<Vertex> VertData, const vk::ResultValue<uint32_t>& ResultValue, const vk::su::SurfaceData& SurfaceData, const int VertexCount);
	void OnRenderFinish(const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device& Device, const vk::su::SwapChainData& SwapChainData, const vk::Queue& GraphicsQueue, const vk::Queue& PresentQueue);
	void NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const VkSampler& Texture, const VkImageView& ImageView);
	void NewOnRendorObjBegin(const VkBuffer& VertexBuffer, const VkBuffer& IndexBuffer);
	void NewOnRendorObj(const int VertAmount, const PushConstant& Uniform);
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
	VkPipelineLayout m_pipeLineLayout;
	VkDescriptorPool m_descriptorPool;
	uint32_t m_descriptorPoolSize;
	vk::DescriptorSet m_descriptorSet;
	VkDescriptorSet m_uniformImage;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;
	unsigned int m_usedModelsAmount{0};\
	std::vector<vk::DescriptorSet> m_descriptorSets;
	VkPipelineCache m_pipelineCache;
	VkPipeline m_pipeline;
	vk::Semaphore imageAcquiredSemaphore;
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

class Renderer : public RendererSpec
{
public:
	Renderer(int Width, int Height);
	void Init() override final;
	void CreateSyncObjects();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateSurface();
	void CreateImageViews();
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
	void PositionCamera(const Vector3& Position, const Vector3& Rotation) override final;
	void Render(const float DeltaTime) override final;
	bool WindowShouldClose() const override final;
	void PollEvents() override final;
	void OnGUI();
	static uint32_t FindMemoryType(const VkPhysicalDevice& PhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void CreateBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CreateTextureImage();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer CommandBuffer);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
	void CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height);
	void CreateTextureImageView();
	VkImageView CreateImageView(VkImage Image, VkFormat Format);
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
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	std::unordered_map<unsigned int, GraphicsModel> m_modelDatas;
	/*
	* RenderTargets
	* Key = render pass
	* Value = mapped pairs
	* key = graphics model
	* value = transform matrix
	*/
	std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::queue<glm::mat4x4>>> m_renderingTargets;
	std::unordered_map<unsigned int, GraphicsRenderPass> m_renderPasses;
	std::unordered_map<unsigned int, VkImage> m_imageDatas;
	std::unordered_map<unsigned int, VkDeviceMemory> m_imageMemory;
	VkImageView m_imageVeiw;
	VkSampler m_sampler;
	//GuiRenderPass m_guiPass;
	glm::mat4x4 m_camMatrix = glm::mat4x4(1);
	ImGui_ImplVulkanH_Window mainWindowData;
	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;
	VkFence m_inFlightFence;
	std::pair<float, float> m_upDateDescriptorTimer{ 0, 1.f };
	bool m_shouldUpdateDescriptor{ true };
};