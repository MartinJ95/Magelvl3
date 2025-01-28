#pragma once

#include "../samples/utils/geometries.hpp"
#include "../samples/utils/math.hpp"
#include "../samples/utils/shaders.hpp"
#include "../samples/utils/utils.hpp"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "Vector.h"

#include <iostream>
#include <thread>
#include <queue>
#include <stack>

constexpr char AppNameC[] = "01_InitInstance";
constexpr char EngineNameC[] = "Vulkan.hpp";

static std::string AppName = "01_InitInstance";
static std::string EngineName = "Vulkan.hpp";

struct GraphicsModel
{
	GraphicsModel(const vk::su::BufferData& Data) : m_vertexBufferData(Data) {}
	GraphicsModel(vk::su::BufferData&& Data) : m_vertexBufferData(std::move(Data)) {}
	void operator=(const vk::su::BufferData& Data) 
	{
		m_vertexBufferData = Data;
	}
	void operator=(vk::su::BufferData&& Data)
	{
		m_vertexBufferData = std::move(Data);
	}
	vk::su::BufferData m_vertexBufferData;
	unsigned int m_renderingPassId;
};

struct GraphicsRenderPass
{
	GraphicsRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device &Device, const vk::su::SurfaceData &SurfaceData, const vk::su::SwapChainData &SwapChainData);
	void SetUniformDataModelViewProjection(const vk::su::SurfaceData& SurfaceData, const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const glm::mat4x4& ModelMatrix, const glm::mat4x4& CamMatrix);
	vk::ResultValue<uint32_t> OnRenderStart(const vk::Device& Device, vk::su::SwapChainData& SwapChainData, vk::CommandBuffer& CommandBuffer, vk::su::SurfaceData& SurfaceData);
	void OnRenderObj(const vk::CommandBuffer& CommandBuffer, const vk::su::BufferData& Data, const vk::ResultValue<uint32_t>& ResultValue, const vk::su::SurfaceData& SurfaceData);
	void OnRenderFinish(const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device& Device, const vk::su::SwapChainData& SwapChainData, const vk::Queue& GraphicsQueue, const vk::Queue& PresentQueue);
	~GraphicsRenderPass();
	std::vector<unsigned int> m_modelID;
	glm::mat4x4 m_mvpcMatrix;
	vk::su::DepthBufferData m_depthBufferData;
	vk::su::BufferData m_uniformBuffer;
	vk::RenderPass m_renderPass;
	vk::ShaderModule m_vertexShaderModule;
	vk::ShaderModule m_fragmentShaderModule;
	std::vector<vk::Framebuffer> m_frameBuffers;
	vk::PipelineLayout m_pipelineLayout;
	vk::DescriptorSetLayout m_descriptorSetLayout;
	vk::DescriptorPool m_descriptorPool;
	vk::DescriptorSet m_descriptorSet;
	std::stack<vk::su::BufferData> m_modelMatrices;
	std::stack<vk::DescriptorSet> m_descriptorSets;
	vk::PipelineCache m_pipelineCache;
	vk::Pipeline m_pipeline;
	vk::Semaphore imageAcquiredSemaphore;
};


class Renderer
{
public:
	Renderer(int Width, int Height);
	void AddToRenderQueue(const unsigned int RenderPass, const Vector3 Pos);
	void Render();
	bool WindowShouldClose();
	void PollEvents();
	~Renderer();
public:
	vk::Instance m_vulkanInstance;
	vk::PhysicalDevice m_physicalDevice;
	vk::su::SurfaceData m_surfaceData;
	std::pair<uint32_t, uint32_t> m_graphicsAndPresentQueueFamilyIndex;
	vk::Device m_device;
	vk::CommandPool m_pool;
	vk::CommandBuffer m_commandBuffer;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
	vk::su::SwapChainData m_swapChainData;
	std::unordered_map<unsigned int, GraphicsModel> m_modelDatas;
	std::unordered_map<unsigned int, std::queue<glm::mat4x4>> m_renderingTargets;
	std::unordered_map<unsigned int, GraphicsRenderPass> m_renderPasses;
	glm::mat4x4 m_camMatrix = glm::mat4x4(1);
};