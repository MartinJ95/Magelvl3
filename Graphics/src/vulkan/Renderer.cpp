#include "Renderer.h"

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
        })
{
    try
    {
        m_renderingTargets.at(0).emplace(vk::su::createModelViewProjectionClipMatrix(
            m_surfaceData.extent));
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
}

void Renderer::AddToRenderQueue(const unsigned int RenderPass, const Vector3 Pos)
{

    glm::mat4x4 transform(1);

    transform = glm::translate(transform, glm::vec3(Pos.x, Pos.y, Pos.z));

    m_renderingTargets.at(RenderPass).emplace(transform);
}

void GraphicsRenderPass::SetUniformDataModelViewProjection(const vk::su::SurfaceData &SurfaceData, const vk::PhysicalDevice& PhysicalDevice, const vk::Device &Device, const glm::mat4x4& ModelMatrix, const glm::mat4x4& CamMatrix)
{
    m_mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(SurfaceData.extent);

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

    float aspectRatio = SurfaceData.extent.height / SurfaceData.extent.width;

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

    m_mvpcMatrix = projectionMatrix * xMatrix * viewMatrix * ModelMatrix;

    //m_mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(SurfaceData.extent);

    m_modelMatrices.emplace(vk::su::BufferData(
        PhysicalDevice,
        Device,
        sizeof(glm::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer));
    //vk::su::copyToDevice(Device, m_uniformBuffer.deviceMemory, m_mvpcMatrix);
    vk::su::copyToDevice(Device, m_modelMatrices.top().deviceMemory, m_mvpcMatrix);

    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);

    //vk::su::updateDescriptorSets(Device, m_descriptorSet, { { vk::DescriptorType::eUniformBuffer, m_uniformBuffer.buffer, VK_WHOLE_SIZE, {} } }, {});
    m_descriptorSets.emplace(Device.allocateDescriptorSets(descriptorSetAllocateInfo).front());
    vk::su::updateDescriptorSets(Device, m_descriptorSets.top(), { { vk::DescriptorType::eUniformBuffer, m_modelMatrices.top().buffer, VK_WHOLE_SIZE, {} } }, {});
}

vk::ResultValue<uint32_t> GraphicsRenderPass::OnRenderStart(const vk::Device &Device, vk::su::SwapChainData &SwapChainData, vk::CommandBuffer &CommandBuffer, vk::su::SurfaceData &SurfaceData)
{
    imageAcquiredSemaphore = Device.createSemaphore(vk::SemaphoreCreateInfo());
    vk::ResultValue<uint32_t> currentBuffer = Device.acquireNextImageKHR(SwapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    assert(currentBuffer.value < m_frameBuffers.size());

    CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 0.2f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderPassBeginInfo renderPassBeginInfo(
        m_renderPass, m_frameBuffers[currentBuffer.value], vk::Rect2D(vk::Offset2D(0, 0), SurfaceData.extent), clearValues);
    CommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    //CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSet, nullptr);

    return currentBuffer;
}

void GraphicsRenderPass::OnRenderObj(const vk::CommandBuffer& CommandBuffer, const vk::su::BufferData& Data, const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::su::SurfaceData& SurfaceData)
{
    CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSets.top(), nullptr);

    CommandBuffer.bindVertexBuffers(0, Data.buffer,{0});

    CommandBuffer.setViewport(
        0, vk::Viewport(0.0f, 0.0f, static_cast<float>(SurfaceData.extent.width), static_cast<float>(SurfaceData.extent.height), 0.0f, 1.0f));
    CommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), SurfaceData.extent));

    CommandBuffer.draw(12 * 3, 1, 0, 0);
}

void GraphicsRenderPass::OnRenderFinish(const vk::ResultValue<uint32_t> &CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device &Device, const vk::su::SwapChainData &SwapChainData, const vk::Queue &GraphicsQueue, const vk::Queue &PresentQueue)
{
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

    while (m_descriptorSets.size() > 0)
    {
        m_modelMatrices.top().clear(Device);
        m_modelMatrices.pop();
        m_descriptorSets.pop();
    }

    vkResetDescriptorPool(Device, m_descriptorPool, {});
}

GraphicsRenderPass::~GraphicsRenderPass()
{
}

void Renderer::Render()
{
    static float rotation{0};
    rotation += 0.001f;
    m_camMatrix = glm::mat4x4(1);
    m_camMatrix = glm::rotate(m_camMatrix, rotation, glm::vec3(0, 1, 0));
    for (auto& it : m_renderingTargets)
    {

        vk::ResultValue<uint32_t> CurrentBuffer = m_renderPasses.at(it.first).OnRenderStart(m_device, m_swapChainData, m_commandBuffer, m_surfaceData);
        while (it.second.size() > 0)
        {
            glm::mat4x4 Transform = it.second.front();
            it.second.pop();

            m_renderPasses.at(it.first).SetUniformDataModelViewProjection(m_surfaceData, m_physicalDevice, m_device, Transform, m_camMatrix);
            m_renderPasses.at(it.first).OnRenderObj(m_commandBuffer, m_modelDatas.at(0).m_vertexBufferData, CurrentBuffer, m_surfaceData);
        }
        m_renderPasses.at(it.first).OnRenderFinish(CurrentBuffer, m_commandBuffer, m_device, m_swapChainData, m_graphicsQueue, m_presentQueue);
    }
  
}

bool Renderer::WindowShouldClose()
{
    return glfwWindowShouldClose(m_surfaceData.window.handle);
}

void Renderer::PollEvents()
{
    glfwPollEvents();
}

Renderer::~Renderer()
{
    /* VULKAN_KEY_END */

    for (auto& it : m_modelDatas)
    {
        it.second.m_vertexBufferData.clear(m_device);
    }
    for (auto& it : m_renderPasses)
    {
        m_device.destroyPipeline(it.second.m_pipeline);
        m_device.destroyPipelineCache(it.second.m_pipelineCache);
        m_device.destroyDescriptorPool(it.second.m_descriptorPool);
        

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
        vk::BufferUsageFlagBits::eUniformBuffer))
{
    vk::su::copyToDevice(Device, m_uniformBuffer.deviceMemory, m_mvpcMatrix);

    m_descriptorSetLayout = vk::su::createDescriptorSetLayout(Device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } });

    m_pipelineLayout = Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), m_descriptorSetLayout));
    /*
    m_renderPass = vk::su::createRenderPass(
        Device, vk::su::pickSurfaceFormat(PhysicalDevice.getSurfaceFormatsKHR(SurfaceData.surface)).format, m_depthBufferData.format);
    */

    glslang::InitializeProcess();

    m_vertexShaderModule = vk::su::createShaderModule(Device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C);

    m_fragmentShaderModule = vk::su::createShaderModule(Device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C);
    glslang::FinalizeProcess();

    m_frameBuffers = vk::su::createFramebuffers(Device, m_renderPass, SwapChainData.imageViews, m_depthBufferData.imageView, SurfaceData.extent);

    //vk::su::copyToDevice(m_device, m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));
    //vk::su::copyToDevice(m_device, m_models.at(0).m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));

    m_descriptorPool = vk::su::createDescriptorPool(Device, { { vk::DescriptorType::eUniformBuffer, 100000 } });
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);

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