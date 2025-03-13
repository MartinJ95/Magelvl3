#include "Renderer.h"
#include "ECS.h"
#include "Transform.h"
#include "PhysicsBase.h"
#include "imgui-1.91.8/misc/cpp/imgui_stdlib.h"

void (*InputFunction)(int, int, int, int) = NULL;

constexpr vk::ApplicationInfo AppInfo(AppNameC, 1, EngineNameC, 1, VK_API_VERSION_1_1);

Renderer::Renderer(int Width, int Height) :
m_vulkanInstance(
    vk::su::createInstance(
        AppName, EngineName, {}, vk::su::getInstanceExtensions())),
    m_physicalDevice(
        m_vulkanInstance.enumeratePhysicalDevices().front()),
    m_surfaceData(
        m_vulkanInstance, AppName, vk::Extent2D(Width, Height)),
    m_graphicsAndPresentQueueFamilyIndex(
        vk::su::findGraphicsAndPresentQueueFamilyIndex(
            m_physicalDevice,
            m_surfaceData.surface)),
    m_device(
        vk::su::createDevice(
            m_physicalDevice,
            m_graphicsAndPresentQueueFamilyIndex.first,
            vk::su::getDeviceExtensions())),
    m_pool(
        m_device.createCommandPool
        (
            vk::CommandPoolCreateInfo
            {
                vk::CommandPoolCreateFlags{VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT},
                m_graphicsAndPresentQueueFamilyIndex.first ,

            }
        )
    ),
    m_commandBuffer(
        m_device.allocateCommandBuffers(
              vk::CommandBufferAllocateInfo(
              m_pool,
              vk::CommandBufferLevel::ePrimary,
              1)
        ).front()
    ),
    m_graphicsQueue(
        m_device.getQueue(
            m_graphicsAndPresentQueueFamilyIndex.first, 0)),
    m_presentQueue(
        m_device.getQueue(
            m_graphicsAndPresentQueueFamilyIndex.second, 0)),
    m_swapChainData(
        m_physicalDevice,
        m_device,
        m_surfaceData.surface,
        m_surfaceData.extent,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        {},
        m_graphicsAndPresentQueueFamilyIndex.first,
        m_graphicsAndPresentQueueFamilyIndex.second),
    m_modelDatas(
        {
            
            {
                0,
                std::move(vk::su::BufferData(m_physicalDevice,
                m_device,
                sizeof(coloredCubeData),
                vk::BufferUsageFlagBits::eVertexBuffer))
            },
            
            {
                1,
                std::move(
                    GraphicsModel(
                        m_physicalDevice, 
                        m_device, 
                        std::move(        
                            std::vector<Vertex>
{
                                    // red face
                                  {glm::vec4{ -1.0f, -1.0f,  1.0f, 1.0f},    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
                                  {glm::vec4(  -1.0f, 1.0f,  1.0f, 1.0f),    {1.0f, 0.0f, 0.0f, 1.0f }},
                                  {glm::vec4(  1.0f, -1.0f,  1.0f, 1.0f),    {1.0f, 0.0f, 0.0f, 1.0f }},
                                  {glm::vec4( 1.0f,  -1.0f,  1.0f, 1.0f),    {1.0f, 0.0f, 0.0f, 1.0f }},
                                  {glm::vec4(  -1.0f,  1.0f,  1.0f, 1.0f),    {1.0f, 0.0f, 0.0f, 1.0f }},
                                  {glm::vec4( 1.0f,  1.0f,  1.0f, 1.0f),    {1.0f, 0.0f, 0.0f, 1.0f }},
                                  // green face
                                  {{ -1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 0.0f, 1.0f }},
                                  // blue face
                                  {{ -1.0f,  1.0f,  1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                {{ -1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  // yellow face
                                  {{  1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f, -1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f, -1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  {{  1.0f, -1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
                                  // magenta face
                                  {{  1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
                                  {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
                                  {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
                                {{ -1.0f,  1.0f, -1.0f, 1.0f}, { 1.0f, 0.0f, 1.0f, 1.0f }},
                                  // cyan face
                                  {{  1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
                                  {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
                                  {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }}
                                  }
                        ),
                 std::move(std::vector<unsigned int>()))
                 )
            }
        }
    ),
    m_renderingTargets(
        {
            {
                0,
                {}
            }
        }
    ),
    m_renderPasses(
        {
            {
                0,
                {
                    m_physicalDevice,
                    m_device,
                    m_surfaceData,
                    m_swapChainData
                }
            }
        }),
    m_guiPass(m_physicalDevice, m_device, m_surfaceData, m_graphicsAndPresentQueueFamilyIndex, m_swapChainData)
{
    try
    {
        m_modelDatas.insert(
            {
                2,
                std::move(
                    GraphicsModel(
                        m_physicalDevice, m_device, GenerateBox(), {}
                    )
                )
            }
        );

        m_modelDatas.insert(
            {
                3,
                std::move(
                    GraphicsModel(
                        m_physicalDevice, m_device, GenerateSphere(), {}
                    )
                )
            }
        );
            
        
        /*
        
        std::vector<Vertex> cubeVertices{
            //red face
            {glm::vec4(-1.0f, -1.0f,  1.0f, 1.0f),    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
            {glm::vec4(1.f, -1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f)},
            {glm::vec4(1.f, -1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f)},
            {glm::vec4(-1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f)},
            {glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f)},
            {glm::vec4(-1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f) },
            //green face
            {glm::vec4( - 1.0f, -1.0f, -1.0f, 1.0f ), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
            {glm::vec4(  1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
            {glm::vec4( - 1.0f,  1.0f, -1.0f, 1.0f),    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
            {glm::vec4( - 1.0f,  1.0f, -1.0f, 1.0f),    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
            {glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
            {glm::vec4(  1.0f,  1.0f, -1.0f, 1.0f),    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f )},
            // blue face
                                  { { -1.0f,  1.0f,  1.0f, 1.0f },    {0.0f, 0.0f, 1.0f, 1.0f } },
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f,  1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                  {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
                                {{ -1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 0.0f, 1.0f, 1.0f }},
            // yellow face
            {{  1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            {{  1.0f, -1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            {{  1.0f, -1.0f,  1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            {{  1.0f, -1.0f, -1.0f, 1.0f},    {1.0f, 1.0f, 0.0f, 1.0f }},
            // magenta face
            {{  1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
            {{ -1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
            {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
            {{  1.0f,  1.0f, -1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
            {{ -1.0f,  1.0f,  1.0f, 1.0f},    {1.0f, 0.0f, 1.0f, 1.0f }},
          {{ -1.0f,  1.0f, -1.0f, 1.0f}, { 1.0f, 0.0f, 1.0f, 1.0f }},
            // cyan face
            {{  1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
            {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
            {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
            {{ -1.0f, -1.0f,  1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
            {{  1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }},
            {{ -1.0f, -1.0f, -1.0f, 1.0f},    {0.0f, 1.0f, 1.0f, 1.0f }}
        };

        std::vector<unsigned int> cubeElements{};

        
        m_modelDatas.emplace(

            1,
            std::move(GraphicsModel(m_physicalDevice, m_device, cubeVertices, cubeElements))
        );
        
        */

        //ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui_ImplGlfw_InitForVulkan(m_surfaceData.window.handle, true);
        ImGui_ImplVulkan_InitInfo info = {};
        info.Instance = m_vulkanInstance;
        info.PhysicalDevice = m_physicalDevice;
        info.Device = m_device;
        info.DescriptorPool = m_guiPass.m_descriptorPool;
        info.DescriptorPoolSize = 0;
        info.RenderPass = m_guiPass.m_renderPass;
        info.MinImageCount = 3;
        info.ImageCount = 3;
        info.QueueFamily = m_graphicsAndPresentQueueFamilyIndex.first;
        info.Queue = m_graphicsQueue;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&info);

        /* temp comm
        m_renderingTargets.at(0).emplace(vk::su::createModelViewProjectionClipMatrix(
            m_surfaceData.extent));
        */
        //vk::Instance instance = vk::su::createInstance(AppName, EngineName, {}, vk::su::getInstanceExtensions());
#if !defined( NDEBUG )
        //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
        vk::su::copyToDevice(m_device, m_modelDatas.at(0).m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));
    }
    catch (vk::SystemError& err)
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    catch (std::exception& err)
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit(-1);
    }
    catch (...)
    {
        std::cout << "unknown error\n";
        exit(-1);
    }
    glfwSetKeyCallback(m_surfaceData.window.handle, KeyCallback);
    mainWindowData.Surface = m_surfaceData.surface;
    //mainWindowData.Swapchain = m_renderPasses.at(0).
    //mainWindowData.RenderPass = m_renderPasses.at(0).m_renderPass;
    //ImGui_ImplVulkanH_CreateOrResizeWindow(m_vulkanInstance, m_physicalDevice, m_device, &mainWindowData, m_graphicsAndPresentQueueFamilyIndex.first, nullptr, m_surfaceData.extent.width, m_surfaceData.extent.height, 2);
}

void Renderer::AddToRenderQueue(const unsigned int RenderPass, const Vector3& Pos, const unsigned int ModelID)
{

    glm::mat4x4 transform(1);

    transform = glm::translate(transform, glm::vec3(Pos.x, Pos.y, Pos.z));

    
    
    if (m_renderingTargets.at(RenderPass).find(ModelID) == m_renderingTargets.at(RenderPass).end())
    {
        m_renderingTargets.at(RenderPass).insert({ ModelID, {} });
    }
    
    
    m_renderingTargets.at(RenderPass).at(ModelID).emplace(transform);
}

void Renderer::PositionCamera(const Vector3& Position, const Vector3& Rotation)
{
    m_camMatrix = glm::mat4x4(1);

    m_camMatrix = glm::translate(m_camMatrix, glm::vec3(Position.x, Position.y, Position.z));
    
    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.x), glm::vec3(1, 0, 0));

    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.y), glm::vec3(0, 1, 0));

    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
    
}

glm::mat4 GraphicsRenderPass::GetViewProjectionMatrix(const vk::su::SurfaceData& SurfaceData, const glm::mat4& CamMatrix)
{
    glm::mat4x4 testMat = vk::su::createModelViewProjectionClipMatrix(SurfaceData.extent);

    glm::mat4x4 viewMatrix = glm::inverse(CamMatrix);

    glm::mat4x4 xMatrix = glm::inverse(
        glm::mat4x4(
            1, 0, 0, 0,
            0, -1, 0, 0,
            0, 0, -1, 0,
            0, 0, 0, 1
        )
    );

    float aspectRatio = SurfaceData.extent.width / SurfaceData.extent.height;

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

void GraphicsRenderPass::SetUniformDataModelViewProjection(const glm::mat4& projectionViewMatrix, const vk::su::SurfaceData &SurfaceData, const vk::PhysicalDevice& PhysicalDevice, const vk::Device &Device, const glm::mat4x4& ModelMatrix, const glm::mat4x4& CamMatrix, const bool ShouldUpdate)
{
    //m_mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(SurfaceData.extent);

    

    m_mvpcMatrix = projectionViewMatrix * ModelMatrix;

    //m_mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(SurfaceData.extent);
    /*
    m_modelMatrices.emplace(vk::su::BufferData(
        PhysicalDevice,
        Device,
        sizeof(glm::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer));
    */
    //vk::su::copyToDevice(Device, m_uniformBuffer.deviceMemory, m_mvpcMatrix);

    unsigned int currentBuffer = 0;
    currentBuffer = m_usedModelsAmount++;
    /*
    for (int i = 0; i < ModelBufferAmount; i++)
    {
        if (m_usedModels.find(i) == m_usedModels.end())
        {
            currentBuffer = i;
            m_usedModels.emplace(i);
            break;
        }
    }
    */
    

    vk::su::copyToDevice(Device, m_modelBuffers.at(currentBuffer).deviceMemory, m_mvpcMatrix);

    //vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);

    //vk::su::updateDescriptorSets(Device, m_descriptorSet, { { vk::DescriptorType::eUniformBuffer, m_uniformBuffer.buffer, VK_WHOLE_SIZE, {} } }, {});
    //m_descriptorSets.emplace(Device.allocateDescriptorSets(descriptorSetAllocateInfo).front());
    //vk::su::updateDescriptorSets(Device, m_descriptorSets.top(), { { vk::DescriptorType::eUniformBuffer, m_modelBuffers.at(currentBuffer).buffer, VK_WHOLE_SIZE, {}}}, {});
    if (ShouldUpdate)
    {
        vk::su::updateDescriptorSets(Device, m_descriptorSets.at(currentBuffer), {{vk::DescriptorType::eUniformBuffer, m_modelBuffers.at(currentBuffer).buffer, VK_WHOLE_SIZE, {}}}, {});
    }
}

vk::ResultValue<uint32_t> GraphicsRenderPass::OnRenderStart(const vk::Device &Device, vk::su::SwapChainData &SwapChainData, vk::CommandBuffer &CommandBuffer, vk::su::SurfaceData &SurfaceData)
{
    imageAcquiredSemaphore = Device.createSemaphore(vk::SemaphoreCreateInfo());
    vk::ResultValue<uint32_t> currentBuffer = Device.acquireNextImageKHR(SwapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    assert(currentBuffer.value < m_frameBuffers.size());

    CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = vk::ClearColorValue(0.2f, 0.2f, 0.4f, 0.2f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderPassBeginInfo renderPassBeginInfo(
        m_renderPass, m_frameBuffers[currentBuffer.value], vk::Rect2D(vk::Offset2D(0, 0), SurfaceData.extent), clearValues);
    CommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    //CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSet, nullptr);

    

    return currentBuffer;
}

void GraphicsRenderPass::OnRenderObj(const vk::CommandBuffer& CommandBuffer, const vk::su::BufferData& Data, const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::su::SurfaceData& SurfaceData, const int VertexCount)
{
    CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSets.at(m_usedModelsAmount-1), nullptr);

    CommandBuffer.bindVertexBuffers(0, Data.buffer,{0});

    /*
    CommandBuffer.setViewport(
        0, vk::Viewport(0.0f, 0.0f, static_cast<float>(SurfaceData.extent.width), static_cast<float>(SurfaceData.extent.height), 0.0f, 1.0f));
    CommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), SurfaceData.extent));
    */
    
    CommandBuffer.draw(VertexCount, 1, 0, 0);
    //CommandBuffer.draw(12 * 3, 1, 0, 0);
}

void GraphicsRenderPass::OnRenderFinish(const vk::ResultValue<uint32_t> &CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device &Device, const vk::su::SwapChainData &SwapChainData, const vk::Queue &GraphicsQueue, const vk::Queue &PresentQueue)
{
    
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

    CommandBuffer.endRenderPass();
    CommandBuffer.end();

    vk::Fence drawFence = Device.createFence(vk::FenceCreateInfo());

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo         submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, CommandBuffer);
    GraphicsQueue.submit(submitInfo, drawFence);

    while (vk::Result::eTimeout == Device.waitForFences(drawFence, VK_TRUE, vk::su::FenceTimeout))
        ;

    vk::Result result = PresentQueue.presentKHR(vk::PresentInfoKHR({}, SwapChainData.swapChain, CurrentBuffer.value));
    switch (result)
    {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR: std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n"; break;
    default: assert(false);  // an unexpected result is returned !
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    Device.waitIdle();

    Device.destroyFence(drawFence);
    Device.destroySemaphore(imageAcquiredSemaphore);

    //m_usedModels.clear();
    m_usedModelsAmount = 0;
    /*
    while (m_descriptorSets.size() > 0)
    {
        //m_modelMatrices.top().clear(Device);
        //m_modelMatrices.pop();
        m_descriptorSets.pop();
    }
    */

    //need to allocate descriptors again after resetting
    //vkResetDescriptorPool(Device, m_descriptorPool, {});
}

void GraphicsRenderPass::CleanUp(const vk::Device& Device)
{
    for (auto& it : m_modelBuffers)
    {
        it.clear(Device);
    }
    //m_modelBuffer.clear(Device);
}

GraphicsRenderPass::~GraphicsRenderPass()
{
    
}

void Renderer::Render(const float DeltaTime)
{
    //static float rotation{0};
    //rotation += 0.001f;
    //m_camMatrix = glm::mat4x4(1);
    //m_camMatrix = glm::rotate(m_camMatrix, rotation, glm::vec3(0, 1, 0));
    
    m_upDateDescriptorTimer.first += DeltaTime;
    if (m_upDateDescriptorTimer.first > m_upDateDescriptorTimer.second)
    {
        m_upDateDescriptorTimer.first = m_upDateDescriptorTimer.first - m_upDateDescriptorTimer.second;
        m_shouldUpdateDescriptor = true;
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    //ImGui::GetIO().DisplaySize = ImVec2(100, 100);
    ImGui::NewFrame();
    ImGui::Begin("first attempt");
    //if (ImGui::BeginMenu("File"))
    //{
    //    if (ImGui::MenuItem("open")) {}
    //}
    //ImGui::EndMenu();
    static glm::vec4 my_color{ 0 };
    if (ImGui::ColorEdit4("Color", (float*)&my_color)) {}
    if (ImGui::SliderFloat("Gravity", &GetPhysicsInstance().m_gravityScale, -10, 10)) {}
    if (ImGui::InputFloat3("World Dir",
        (float*) & GetPhysicsInstance().m_downVec)) {
    }
    if (ImGui::InputFloat3("CamPos", (float*)&GetEcsInstance().FindComponent<Transform>(0).GetPositionRef())) {}
    ImGui::End();
    ImGui::ShowDemoWindow();

    ImGui::Begin("Entities");

    if (ImGui::Button("AddNewEntity"))
    {
        GetEcsInstance().AddNewEntity();
    }

    for (auto& ent : GetEcsInstance().m_entities)
    {

        std::string entLabel = std::to_string(ent.first);

        if (ImGui::InputText(entLabel.c_str(), &ent.second, flags))
        {

        }
        if (ImGui::Button((std::string("RemoveEntity") + std::to_string(ent.first)).c_str()))
        {
            EcsInstance->MarkEntityForRemoval(ent.first);
            
        }

        if (ImGui::BeginCombo((entLabel + "combo").c_str(), ent.second.c_str()))
        {
            ECS& ecsystem = GetEcsInstance();

                std::unordered_map<unsigned int, Component*> comps = ecsystem.GetAllComponentsOfEntity(ent.first);

                if (ImGui::BeginCombo("component addition", "Add Component"))
                {
                    // add component based on registered components

                    for (auto& registeredComp : EcsInstance->m_componentNames)
                    {
                        if (ImGui::Button(registeredComp.second.c_str()))
                        {
                            EcsInstance->AddComponent(ent.first, registeredComp.first);
                        }
                    }
                    ImGui::EndCombo();
                }

                for (auto& comp : comps)
                {

                    std::vector<std::pair<ComponentDecompositionTypes, std::string>> types;
                    comp.second->GetDecompositions(types);

                    unsigned int offset = sizeof(char)*8;
                    
                    while (!types.empty())
                    {
                        char* data = (char*)comp.second;
                        data += offset;

                        unsigned int CompOffset = 0;

                        switch (types.back().first)
                        {
                        case ComponentDecompositionTypes::eInt:
                            ImGui::InputInt((EcsInstance->m_componentNames.at(comp.first) + entLabel + "o" + std::to_string(offset) + types.back().second).c_str(), (int*)data);
                            offset += (sizeof(int) / sizeof(char));
                            break;
                        case ComponentDecompositionTypes::eBool:
                            break;
                        case ComponentDecompositionTypes::eFloat3:
                            ImGui::InputFloat3((EcsInstance->m_componentNames.at(comp.first) + entLabel + "o" + std::to_string(offset) + types.back().second).c_str(), (float*)data);
                            offset += (sizeof(float) / sizeof(char)) * 3;
                            break;
                        case ComponentDecompositionTypes::ePad:
                            offset += (sizeof(char));
                            default:
                                break;
                        }
                        CompOffset++;
                        types.pop_back();
                    }

                    if (ImGui::Button(((EcsInstance->m_componentNames.at(comp.first) + "removal").c_str())))
                    {
                        GetEcsInstance().RemoveComponent(comp.second->GetEntity(), comp.first);
                    }
                    
                }

            ImGui::EndCombo();
        }
    }
    ImGui::End();
    ImGui::Render();
    ImGui::EndFrame();
    
    //works but doesnt at same time
    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commandBuffer);
    // 
    
    for (auto& it : m_renderingTargets)
    {
        for (auto& it1 : it.second)
        {

            int lastUsed = -1;
            int Current = 0;

            vk::ResultValue<uint32_t> CurrentBuffer = m_renderPasses.at(it.first).OnRenderStart(m_device, m_swapChainData, m_commandBuffer, m_surfaceData);

            glm::mat4 viewProjection = m_renderPasses.at(it.first).GetViewProjectionMatrix(m_surfaceData, m_camMatrix);

            m_commandBuffer.setViewport(
                0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_surfaceData.extent.width), static_cast<float>(m_surfaceData.extent.height), 0.0f, 1.0f));

            m_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_surfaceData.extent));

            while (it1.second.size() > 0)
            {
                glm::mat4x4 Transform = it1.second.front();
                
                it1.second.pop();

                m_renderPasses.at(it.first).SetUniformDataModelViewProjection(viewProjection, m_surfaceData, m_physicalDevice, m_device, Transform, m_camMatrix, m_shouldUpdateDescriptor);

            

                m_renderPasses.at(it.first).OnRenderObj(m_commandBuffer, m_modelDatas.at(it1.first).m_vertexBufferData, CurrentBuffer, m_surfaceData, m_modelDatas.at(it1.first).m_vertices.size());
            }
        
            m_renderPasses.at(it.first).OnRenderFinish(CurrentBuffer, m_commandBuffer, m_device, m_swapChainData, m_graphicsQueue, m_presentQueue);
        }
    m_shouldUpdateDescriptor = false;
    }
    
}

bool Renderer::WindowShouldClose() const
{
    return glfwWindowShouldClose(m_surfaceData.window.handle);
}

void Renderer::PollEvents()
{
    glfwPollEvents();
}

void Renderer::OnGUIStart()
{
    //ImGui_ImplVulkan_InitInfo info;
    //ImGui_ImplVulkan_Init(&info);
    /*
    ImGui::NewFrame();
    ImGui::Begin("first attempt");
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("open")) {}
    }
    static glm::vec4 my_color{ 0 };
    if (ImGui::ColorEdit4("Color", (float*)&my_color)) {}
    ImGui::EndMenuBar();
    ImGui::End();
    ImGui::EndFrame();
    */
    
}

Renderer::~Renderer()
{
    /* VULKAN_KEY_END */

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto& it : m_modelDatas)
    {
        it.second.m_vertexBufferData.clear(m_device);
    }
    for (auto& it : m_renderPasses)
    {
        vkResetDescriptorPool(m_device, it.second.m_descriptorPool, {});
        m_device.destroyPipeline(it.second.m_pipeline);
        m_device.destroyPipelineCache(it.second.m_pipelineCache);
        m_device.destroyDescriptorPool(it.second.m_descriptorPool);
        it.second.CleanUp(m_device);
        

        for (auto &framebuffer : it.second.m_frameBuffers)
        {
            m_device.destroyFramebuffer(framebuffer);
        }
        m_device.destroyShaderModule(it.second.m_fragmentShaderModule);
        m_device.destroyShaderModule(it.second.m_vertexShaderModule);
        m_device.destroyRenderPass(it.second.m_renderPass);
        m_device.destroyPipelineLayout(it.second.m_pipelineLayout);
        m_device.destroyDescriptorSetLayout(it.second.m_descriptorSetLayout);
        it.second.m_uniformBuffer.clear(m_device);
        it.second.m_depthBufferData.clear(m_device);
    }
    m_swapChainData.clear(m_device);
    m_device.destroyCommandPool(m_pool);
    m_device.destroy();
    m_vulkanInstance.destroySurfaceKHR(m_surfaceData.surface);
#if !defined( NDEBUG )
    //instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    m_vulkanInstance.destroy();
}

GraphicsRenderPass::GraphicsRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device &Device, const vk::su::SurfaceData &SurfaceData, const vk::su::SwapChainData& SwapChainData) :
    m_mvpcMatrix(
        vk::su::createModelViewProjectionClipMatrix(
            SurfaceData.extent)
    ),
    m_depthBufferData(
        vk::su::DepthBufferData(
            PhysicalDevice,
            Device,
            vk::Format::eD16Unorm,
            SurfaceData.extent)),
    m_renderPass(
        vk::su::createRenderPass(
            Device,
            vk::su::pickSurfaceFormat(
                PhysicalDevice.getSurfaceFormatsKHR(
                    SurfaceData.surface)).format,
                    m_depthBufferData.format)
    ),
    imageAcquiredSemaphore(),
    m_uniformBuffer(vk::su::BufferData(
        PhysicalDevice,
        Device,
        sizeof(glm::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer)),
    m_modelBuffers()
{
    m_modelBuffers.reserve(ModelBufferAmount);
    m_descriptorSets.reserve(ModelBufferAmount);
    /*
    m_modelBuffers.fill(vk::su::BufferData(
        PhysicalDevice,
        Device,
        sizeof(glm::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer));
    */
    vk::su::copyToDevice(Device, m_uniformBuffer.deviceMemory, m_mvpcMatrix);

    m_descriptorSetLayout = vk::su::createDescriptorSetLayout(Device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } });

    m_pipelineLayout = Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), m_descriptorSetLayout));
    /*
    m_renderPass = vk::su::createRenderPass(
        Device, vk::su::pickSurfaceFormat(PhysicalDevice.getSurfaceFormatsKHR(SurfaceData.surface)).format, m_depthBufferData.format);
    */

    glslang::InitializeProcess();

    m_vertexShaderModule = vk::su::createShaderModule(Device, vk::ShaderStageFlagBits::eVertex, VertexShader);

    m_fragmentShaderModule = vk::su::createShaderModule(Device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C);
    glslang::FinalizeProcess();

    m_frameBuffers = vk::su::createFramebuffers(Device, m_renderPass, SwapChainData.imageViews, m_depthBufferData.imageView, SurfaceData.extent);

    //vk::su::copyToDevice(m_device, m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));
    //vk::su::copyToDevice(m_device, m_models.at(0).m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));
    m_descriptorPoolSize = ModelBufferAmount;
    m_descriptorPool = vk::su::createDescriptorPool(Device, { { vk::DescriptorType::eUniformBuffer, ModelBufferAmount + 100 } });
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);
    
    //m_descriptorSets.emplace_back(Device.allocateDescriptorSets(descriptorSetAllocateInfo).front());
    for (int i = 0; i < ModelBufferAmount; i++)
    {
        m_modelBuffers.emplace_back(vk::su::BufferData(
            PhysicalDevice,
            Device,
            sizeof(glm::mat4),
            vk::BufferUsageFlagBits::eUniformBuffer));
        m_descriptorSets.emplace_back(Device.allocateDescriptorSets(descriptorSetAllocateInfo).front());
    }

    m_descriptorSet = Device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

    vk::su::updateDescriptorSets(Device, m_descriptorSet, { { vk::DescriptorType::eUniformBuffer, m_uniformBuffer.buffer, VK_WHOLE_SIZE, {} } }, {});

    m_pipelineCache = Device.createPipelineCache(vk::PipelineCacheCreateInfo());

    m_pipeline = vk::su::createGraphicsPipeline(Device,
        m_pipelineCache,
        std::make_pair(m_vertexShaderModule, nullptr),
        std::make_pair(m_fragmentShaderModule, nullptr),
        sizeof(coloredCubeData[0]),
        { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32B32A32Sfloat, 16 } },
        vk::FrontFace::eClockwise,
        true,
        m_pipelineLayout,
        m_renderPass);
}

GuiRenderPass::GuiRenderPass(const vk::PhysicalDevice& PhysicalDevice, const vk::Device& Device, const vk::su::SurfaceData& SurfaceData, std::pair<uint32_t, uint32_t>& GraphicsAndPresentQueueFamilyIndex, const vk::su::SwapChainData& SwapChainData) :
    m_depthBufferData(vk::su::DepthBufferData(
        PhysicalDevice,
        Device,
        vk::Format::eD16Unorm,
        SurfaceData.extent)),
    m_renderPass(vk::su::createRenderPass(
        Device,
        vk::su::pickSurfaceFormat(
            PhysicalDevice.getSurfaceFormatsKHR(
                SurfaceData.surface)).format,
        m_depthBufferData.format)
    ),
    m_commandBuffer(
        Device.allocateCommandBuffers(
            vk::CommandBufferAllocateInfo(
                Device.createCommandPool
                (
                    vk::CommandPoolCreateInfo
                    {
                        vk::CommandPoolCreateFlags{VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT},
                        GraphicsAndPresentQueueFamilyIndex.first ,
                    }
                    ),
                vk::CommandBufferLevel::ePrimary,
                1)
        ).front()
    )
{
    ImGui::CreateContext();
    m_descriptorPool = vk::su::createDescriptorPool(Device, { { vk::DescriptorType::eUniformBuffer, ModelBufferAmount + 100 } });
    m_frameBuffers = vk::su::createFramebuffers(Device, m_renderPass, SwapChainData.imageViews, m_depthBufferData.imageView, SurfaceData.extent);
    /*
    m_commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    ImGui_ImplVulkan_CreateFontsTexture();
    m_commandBuffer.end();
    */

}
