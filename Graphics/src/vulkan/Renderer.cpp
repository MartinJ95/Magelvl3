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
        m_device.createCommandPool(
            {
                {},
                m_graphicsAndPresentQueueFamilyIndex.first })),
                m_commandBuffer(
                    m_device.allocateCommandBuffers(
                        vk::CommandBufferAllocateInfo(
                            m_pool,
                            vk::CommandBufferLevel::ePrimary,
                            1)).front()
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
    m_depthBufferData(
        vk::su::DepthBufferData(
            m_physicalDevice,
            m_device,
            vk::Format::eD16Unorm,
            m_surfaceData.extent)),
    m_uniformBuffer(vk::su::BufferData(
        m_physicalDevice,
        m_device,
        sizeof(glm::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer)),
    m_vertexBufferData(
        vk::su::BufferData(
            m_physicalDevice,
            m_device,
            sizeof(coloredCubeData),
            vk::BufferUsageFlagBits::eVertexBuffer)),
    m_mvpcMatrix(
        vk::su::createModelViewProjectionClipMatrix(
            m_surfaceData.extent)
    )
{
    try
    {
        //vk::Instance instance = vk::su::createInstance(AppName, EngineName, {}, vk::su::getInstanceExtensions());
#if !defined( NDEBUG )
        //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
        vk::su::copyToDevice(m_device, m_uniformBuffer.deviceMemory, m_mvpcMatrix);

        m_descriptorSetLayout = vk::su::createDescriptorSetLayout(m_device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } });

        m_pipelineLayout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), m_descriptorSetLayout));

        m_renderPass = vk::su::createRenderPass(
            m_device, vk::su::pickSurfaceFormat(m_physicalDevice.getSurfaceFormatsKHR(m_surfaceData.surface)).format, m_depthBufferData.format);

        glslang::InitializeProcess();

        m_vertexShaderModule = vk::su::createShaderModule(m_device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C);

        m_fragmentShaderModule = vk::su::createShaderModule(m_device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C);
        glslang::FinalizeProcess();


        m_frameBuffers = vk::su::createFramebuffers(m_device, m_renderPass, m_swapChainData.imageViews, m_depthBufferData.imageView, m_surfaceData.extent);

        vk::su::copyToDevice(m_device, m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));

        m_descriptorPool = vk::su::createDescriptorPool(m_device, { { vk::DescriptorType::eUniformBuffer, 1 } });
        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);

        m_descriptorSet = m_device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

        vk::su::updateDescriptorSets(m_device, m_descriptorSet, { { vk::DescriptorType::eUniformBuffer, m_uniformBuffer.buffer, VK_WHOLE_SIZE, {} } }, {});

        m_pipelineCache = m_device.createPipelineCache(vk::PipelineCacheCreateInfo());

        m_pipeline = vk::su::createGraphicsPipeline(m_device,
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

void Renderer::Render()
{
    /* VULKAN_KEY_START */

      // Get the index of the next available swapchain image:
    vk::Semaphore             imageAcquiredSemaphore = m_device.createSemaphore(vk::SemaphoreCreateInfo());
    vk::ResultValue<uint32_t> currentBuffer = m_device.acquireNextImageKHR(m_swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    assert(currentBuffer.value < m_frameBuffers.size());

    m_commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 0.2f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderPassBeginInfo renderPassBeginInfo(
        m_renderPass, m_frameBuffers[currentBuffer.value], vk::Rect2D(vk::Offset2D(0, 0), m_surfaceData.extent), clearValues);
    m_commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSet, nullptr);

    m_commandBuffer.bindVertexBuffers(0, m_vertexBufferData.buffer, { 0 });
    m_commandBuffer.setViewport(
        0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_surfaceData.extent.width), static_cast<float>(m_surfaceData.extent.height), 0.0f, 1.0f));
    m_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_surfaceData.extent));

    m_commandBuffer.draw(12 * 3, 1, 0, 0);
    m_commandBuffer.endRenderPass();
    m_commandBuffer.end();

    vk::Fence drawFence = m_device.createFence(vk::FenceCreateInfo());

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo         submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, m_commandBuffer);
    m_graphicsQueue.submit(submitInfo, drawFence);

    while (vk::Result::eTimeout == m_device.waitForFences(drawFence, VK_TRUE, vk::su::FenceTimeout))
        ;

    vk::Result result = m_presentQueue.presentKHR(vk::PresentInfoKHR({}, m_swapChainData.swapChain, currentBuffer.value));
    switch (result)
    {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR: std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n"; break;
    default: assert(false);  // an unexpected result is returned !
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    m_device.waitIdle();

    m_device.destroyFence(drawFence);
    m_device.destroySemaphore(imageAcquiredSemaphore);
}

Renderer::~Renderer()
{
    /* VULKAN_KEY_END */

    m_device.destroyPipeline(m_pipeline);
    m_device.destroyPipelineCache(m_pipelineCache);
    m_device.destroyDescriptorPool(m_descriptorPool);
    m_vertexBufferData.clear(m_device);
    for (auto framebuffer : m_frameBuffers)
    {
        m_device.destroyFramebuffer(framebuffer);
    }
    m_device.destroyShaderModule(m_fragmentShaderModule);
    m_device.destroyShaderModule(m_vertexShaderModule);
    m_device.destroyRenderPass(m_renderPass);
    m_device.destroyPipelineLayout(m_pipelineLayout);
    m_device.destroyDescriptorSetLayout(m_descriptorSetLayout);
    m_uniformBuffer.clear(m_device);
    m_depthBufferData.clear(m_device);
    m_swapChainData.clear(m_device);
    m_device.destroyCommandPool(m_pool);
    m_device.destroy();
    m_vulkanInstance.destroySurfaceKHR(m_surfaceData.surface);
#if !defined( NDEBUG )
    //instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    m_vulkanInstance.destroy();
}
