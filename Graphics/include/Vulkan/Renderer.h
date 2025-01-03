#pragma once

#include "../samples/utils/geometries.hpp"
#include "../samples/utils/math.hpp"
#include "../samples/utils/shaders.hpp"
#include "../samples/utils/utils.hpp"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"

#include <iostream>
#include <thread>

constexpr char AppNameC[] = "01_InitInstance";
constexpr char EngineNameC[] = "Vulkan.hpp";

static std::string AppName = "01_InitInstance";
static std::string EngineName = "Vulkan.hpp";

class Renderer
{
public:
	Renderer(int Width, int Height);
	void Render();
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
	vk::su::DepthBufferData m_depthBufferData;
	vk::su::BufferData m_uniformBuffer;
	vk::su::BufferData m_vertexBufferData;
	glm::mat4x4 m_mvpcMatrix;
	vk::DescriptorSetLayout m_descriptorSetLayout;
	vk::PipelineLayout m_pipelineLayout;
	vk::RenderPass m_renderPass;
	vk::ShaderModule m_vertexShaderModule;
	vk::ShaderModule m_fragmentShaderModule;
	std::vector<vk::Framebuffer> m_frameBuffers;
	vk::DescriptorPool m_descriptorPool;
	vk::DescriptorSet m_descriptorSet;
	vk::PipelineCache m_pipelineCache;
	vk::Pipeline m_pipeline;
};