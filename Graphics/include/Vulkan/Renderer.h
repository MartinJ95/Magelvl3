#pragma once

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

constexpr char AppNameC[] = "01_InitInstance";
constexpr char EngineNameC[] = "Vulkan.hpp";

static std::string AppName = "01_InitInstance";
static std::string EngineName = "Vulkan.hpp";

extern void (*InputFunction)(int, int, int, int);

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	InputFunction(key, scancode, action, mods);
}

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

constexpr unsigned int ModelBufferAmount = 50*50*50+10;

struct GraphicsRenderPass
{
	GraphicsRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device &Device, const vk::su::SurfaceData &SurfaceData, const vk::su::SwapChainData &SwapChainData);
	glm::mat4 GetViewProjectionMatrix(const vk::su::SurfaceData& SurfaceData, const glm::mat4& CamMatrix);
	void SetUniformDataModelViewProjection(const glm::mat4& projectionViewMatrix,const vk::su::SurfaceData& SurfaceData, const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const glm::mat4x4& ModelMatrix, const glm::mat4x4& CamMatrix, const bool ShouldUpdate);
	vk::ResultValue<uint32_t> OnRenderStart(const vk::Device& Device, vk::su::SwapChainData& SwapChainData, vk::CommandBuffer& CommandBuffer, vk::su::SurfaceData& SurfaceData);
	void OnRenderObj(const vk::CommandBuffer& CommandBuffer, const vk::su::BufferData& Data, const vk::ResultValue<uint32_t>& ResultValue, const vk::su::SurfaceData& SurfaceData);
	void OnRenderFinish(const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device& Device, const vk::su::SwapChainData& SwapChainData, const vk::Queue& GraphicsQueue, const vk::Queue& PresentQueue);
	void CleanUp(const vk::Device& Device);
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
	uint32_t m_descriptorPoolSize;
	vk::DescriptorSet m_descriptorSet;
	std::vector<vk::su::BufferData> m_modelBuffers;
	unsigned int m_usedModelsAmount{0};
	std::vector<vk::DescriptorSet> m_descriptorSets;
	vk::PipelineCache m_pipelineCache;
	vk::Pipeline m_pipeline;
	vk::Semaphore imageAcquiredSemaphore;
};

struct GuiRenderPass
{
	GuiRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const vk::su::SurfaceData& SurfaceData, std::pair<uint32_t, uint32_t>& GraphicsAndPresentQueueFamilyIndex, const vk::su::SwapChainData& SwapChainData);
	vk::su::DepthBufferData m_depthBufferData;
	vk::DescriptorPool m_descriptorPool;
	vk::RenderPass m_renderPass;
	vk::CommandBuffer m_commandBuffer;
	std::vector<vk::Framebuffer> m_frameBuffers;
};


class Renderer : public RendererSpec
{
public:
	Renderer(int Width, int Height);
	void AddToRenderQueue(const unsigned int RenderPass, const Vector3 Pos) override final;
	void PositionCamera(const Vector3& Position, const Vector3& Rotation) override final;
	void Render(const float DeltaTime) override final;
	bool WindowShouldClose() const override final;
	void PollEvents() override final;
	void OnGUIStart();
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
	GuiRenderPass m_guiPass;
	glm::mat4x4 m_camMatrix = glm::mat4x4(1);
	ImGui_ImplVulkanH_Window mainWindowData;
	std::pair<float, float> m_upDateDescriptorTimer{ 0, 1.f };
	bool m_shouldUpdateDescriptor{ true };
};