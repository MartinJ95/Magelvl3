#include "Renderer.h"
#include "ECS.h"
#include "Transform.h"
#include "PhysicsBase.h"
#include "misc/cpp/imgui_stdlib.h"
#include <map>
#include <set>

#include <glm/common.hpp>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void (*InputFunction)(int, int, int, int) = NULL;

//constexpr VkApplicationInfo AppInfo(AppNameC, 1, EngineNameC, 1, VK_API_VERSION_1_1);

Renderer::Renderer(int Width, int Height) :
   
    m_pool(
     ),
    m_modelDatas(),
    m_renderingTargets(
        {
            {
                0,
                {}
            }
        }
    ),
    m_renderPasses(
        /*
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
        }
        */
        ),
    m_width(Width),
    m_height(Height)
    //m_guiPass(m_physicalDevice, m_device, m_surfaceData, m_graphicsAndPresentQueueFamilyIndex, m_swapChainData)
{
    try
    {
        
       

        

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        /* temp comm
        m_renderingTargets.at(0).emplace(vk::su::createModelViewProjectionClipMatrix(
            m_surfaceData.extent));
        */
        //vk::Instance instance = vk::su::createInstance(AppName, EngineName, {}, vk::su::getInstanceExtensions());
#if !defined( NDEBUG )
        //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
        //vk::su::copyToDevice(m_device, m_modelDatas.at(0).m_vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));
    }
    /*
    catch (vk::SystemError& err)
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    */
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
    
    //mainWindowData.Swapchain = m_renderPasses.at(0).
    //mainWindowData.RenderPass = m_renderPasses.at(0).m_renderPass;
    //ImGui_ImplVulkanH_CreateOrResizeWindow(m_vulkanInstance, m_physicalDevice, m_device, &mainWindowData, m_graphicsAndPresentQueueFamilyIndex.first, nullptr, m_surfaceData.extent.width, m_surfaceData.extent.height, 2);
}

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void Renderer::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);

    m_extents.width = m_width;
    m_extents.height = m_height;

    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();

    CreateSingleUseCommandPoolAndBuffer();

    CreateShadowMapImageViews();
    CreateViewportImageVies();

    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();

    GraphicsRenderPassOptions options;
    options.FragmentShader = "sFrag.spv";
    options.VertexShader = "sVert.spv";
    options.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    options.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    options.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    options.IsShadowMapper = true;


    CreateDepthResources();

    

    //m_renderPasses.emplace(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple());
    //m_renderPasses.emplace(std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple());
    
    for (int i = 0; i < 3; i++)
    {
        m_renderPasses.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple());
        m_shadowPasses.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple());
    }

    //terrain shadow pass
    //m_renderPasses[0].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_swapChainImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);
    m_shadowPasses[0].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_shadowMapImageViews, m_shadowMapDepthImageView, m_surface, options);

    options.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    options.LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    //normal pass shadow
    //m_renderPasses[1].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_swapChainImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);
    m_shadowPasses[1].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_shadowMapImageViews, m_shadowMapDepthImageView, m_surface, options);
    m_shadowPasses[2].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_shadowMapImageViews, m_shadowMapDepthImageView, m_surface, options);
    options.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    options.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    options.IsShadowMapper = false;

    options.FragmentShader = "frag.spv";
    options.VertexShader = "vert.spv";

    //terrain normal pass
    m_renderPasses[0].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_viewportImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);

    options.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    options.LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    //normal pass non shadow
    m_renderPasses[1].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_viewportImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);
    m_renderPasses[2].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_viewportImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);

    options.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    m_guiPass.Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_swapChainImageViews, m_shadowMapImageViews, m_shadowMapDepthImageView, depthImageView, m_surface, options);
    //m_terrain.reserve(3);

    PositionLight(Vector3(0, 60, 0), Vector3(55, 0, 0));
    
    for (int i = 0; i < m_renderPasses.size(); i++)
    {
        if (m_renderingTargets.find(i) == m_renderingTargets.end())
        {
            //m_renderingTargets.emplace(i);
            m_renderingTargets[i];
        }
    }

    m_terrain.emplace(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(m_physicalDevice, m_device, 200));


    CreateSyncObjects();

    m_modelDatas.emplace(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple());
    m_modelDatas.emplace(std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple());

    m_modelDatas.at(0).SubModels.emplace(
        std::piecewise_construct, std::forward_as_tuple(0),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            GenerateBox(),
            std::vector<uint16_t>())

    );
    m_modelDatas.at(1).SubModels.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(1),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            GenerateSphere(),
            std::vector<uint16_t>())

    );
    
    m_modelDatas.emplace(std::piecewise_construct, std::forward_as_tuple(2), std::forward_as_tuple());
    m_modelDatas.emplace(std::piecewise_construct, std::forward_as_tuple(3), std::forward_as_tuple());
    
    Mesh loader;
    std::vector<std::vector<Vertex>> verts{};
    std::vector<std::vector<uint16_t>> elements{};
    loader.LoadFromObj("Mug.obj", verts, elements);
    for (int i = 0; i < verts.size(); i++)
    {
        m_modelDatas.at(2).SubModels.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(i),
            std::forward_as_tuple(m_physicalDevice,
                m_device,
                std::move(verts[i]),
                std::move(elements[i]))
        );

    }
    /*
    m_modelDatas.at(2).SubModels.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(0),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            std::move(verts),
            std::move(elements))
    );
    */
    verts.clear();
    elements.clear();

    
    std::vector<std::vector<Vertex>> verts2{};
    std::vector<std::vector<uint16_t>> elements2{};
    loader.LoadFromObj("tree1.obj", verts2, elements2);
    for (int i = 0; i < verts2.size(); i++)
    {
        m_modelDatas.at(3).SubModels.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(i),
            std::forward_as_tuple(m_physicalDevice,
                m_device,
                std::move(verts2[i]),
                std::move(elements2[i]))
        );

    }
    

    /*
    m_modelDatas.at(3).SubModels.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(0),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            std::move(verts2),
            std::move(elements2)
        )
    );
    */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    ImGui_ImplVulkan_InitInfo info = {};
    info.Instance = m_vulkanInstance;
    info.PhysicalDevice = m_physicalDevice;
    info.Device = m_device;
    //info.DescriptorPool = m_renderPasses[0].m_descriptorPool;
    info.DescriptorPoolSize = 8;
    info.RenderPass = m_renderPasses[0].m_renderPass;
    info.MinImageCount = 3;
    info.ImageCount = 3;
    info.QueueFamily = m_graphicsAndPresentQueueFamilyIndex.first;
    info.Queue = m_graphicsQueue;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.CheckVkResultFn = check_vk_result;


    //will bring back
    ImGui_ImplVulkan_Init(&info);

    m_viewportDescriptorSet = ImGui_ImplVulkan_AddTexture(m_samplers[0], m_viewportImageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    for (auto& model : m_modelDatas)
    {
        for(auto& subModel : model.second.SubModels)
        {
            for (int i = 0; i < subModel.second.m_vertices.size(); i = i + 3)
            {
                subModel.second.m_elements.emplace_back(i);
                subModel.second.m_elements.emplace_back(i + 1);
                subModel.second.m_elements.emplace_back(i + 2);
            }
            subModel.second.InitIndices(m_physicalDevice, m_device);
        }
        
    }

    //m_modelDatas.at(0).InitIndices(m_physicalDevice, m_device);
    //m_modelDatas.at(1).InitIndices(m_physicalDevice, m_device);

    glfwSetKeyCallback(m_window, KeyCallback);
    mainWindowData.Surface = m_surface;
}

void Renderer::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void Renderer::CreateSingleUseCommandPoolAndBuffer()
{
    QueueFamilyIndices queueFamilyIndices = Renderer::FindQueueFamilies(m_physicalDevice, m_surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, indices.GraphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.PresentFamily.value(), 0, &m_presentQueue);
}

void Renderer::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

    if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
        imageCount = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_surface);
    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily != indices.PresentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    m_shadowMapImageViews.resize(imageCount);
    m_viewportImages.resize(imageCount);
    m_viewportImageViews.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void Renderer::CreateSurface()
{
    /*
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(m_window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(m_vulkanInstance, &createInfo, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    */
    if (glfwCreateWindowSurface(m_vulkanInstance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }

}

void Renderer::CreateImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void Renderer::CreateShadowMapImageViews()
{
    m_shadowMapImages.resize(m_shadowMapImageViews.size());
    m_shadowMapImagesMemory.resize(m_shadowMapImageViews.size());

    // make images
    //int texWidth, texHeight, texChannels;

    
    /*
    stbi_uc* pixels = stbi_load("grass.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(m_physicalDevice, m_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free(pixels);
    */

    for (int i = 0; i < m_shadowMapImages.size(); i++)
    {
        CreateImage(m_extents.width, m_extents.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_shadowMapImages[i], m_shadowMapImagesMemory[i]);

        //TransitionImageLayout(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        //CopyBufferToImage(stagingBuffer, m_shadowMapImages[i], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        //TransitionImageLayout(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    //m_imageDatas.emplace(0, nullptr);
    //m_imageMemory.emplace(0, nullptr);

    

    //vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    //vkFreeMemory(m_device, stagingBufferMemory, nullptr);

    //finish images

    //make image views

    for (int i = 0; i < m_shadowMapImages.size(); i++)
    {
        m_shadowMapImageViews[i] = CreateImageView(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    //finish image views

    //make sampler

    VkFormat depthFormat = FindDepthFormat(m_physicalDevice);

    CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_shadowMapDepthImage, m_shadowMapDepthMemory);

    m_shadowMapDepthImageView = CreateImageView(m_shadowMapDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(m_shadowMapDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::CreateViewportImageVies()
{
    m_viewportImageMemory.resize(m_viewportImageViews.size());

    //make images

    for (int i = 0; i < m_shadowMapImages.size(); i++)
    {
        CreateImage(m_extents.width, m_extents.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_viewportImages[i], m_viewportImageMemory[i]);

        //TransitionImageLayout(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        //CopyBufferToImage(stagingBuffer, m_shadowMapImages[i], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        //TransitionImageLayout(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    //make image views

    for (int i = 0; i < m_shadowMapImages.size(); i++)
    {
        m_viewportImageViews[i] = CreateImageView(m_shadowMapImages[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }


}

void Renderer::CreateGraphicsPipeline()
{

}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* PCallbackData, void* PUserData)
{
    std::cerr << "validation layer: " << PCallbackData->pMessage << std::endl;

    if (MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        // Message is important enough to show
    }

    return VK_FALSE;
}

VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
    if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return Capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes)
{
    for (const auto& availablePresentMode : AvailablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
{
    for (const auto& availableFormat : AvailableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }



    return AvailableFormats[0];
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice& Device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, m_surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, m_surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, m_surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
{
    CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    CreateInfo.pfnUserCallback = DebugCallback;
}

std::vector<const char*> Renderer::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Renderer::SetupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);
    /*
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr; // Optional
    */

    if (CreateDebugUtilsMessengerEXT(m_vulkanInstance, &createInfo, nullptr, debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance Instance, const VkDebugUtilsMessengerCreateInfoEXT* PCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(Instance, PCreateInfo, pAllocator, &pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance Instance, VkDebugUtilsMessengerEXT DebugMessenger, const VkAllocationCallbacks* Allocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(Instance, debugMessenger, Allocator);
    }
}

int Renderer::RateDeviceSuitability(VkPhysicalDevice Device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(Device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(Device, &deviceFeatures);

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}

bool Renderer::IsDeviceSuitable(VkPhysicalDevice Device)
{
    QueueFamilyIndices indices = FindQueueFamilies(Device, m_surface);

    bool extensionsSupported = CheckDeviceExtensionSupport(Device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(Device);
        swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
    }

    if (!indices.IsComplete() || !extensionsSupported || !swapChainAdequate)
    {
        return false;
    }


    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(Device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(Device, &deviceFeatures);


    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader;
}

bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice Device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Renderer::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

QueueFamilyIndices Renderer::FindQueueFamilies(const VkPhysicalDevice& Device, const VkSurfaceKHR& Surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, queueFamilies.data());

    int i = 0;

    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &presentSupport);

            if (presentSupport) {
                indices.PresentFamily = i;
            }

            indices.GraphicsFamily = i;
        }

        if (indices.IsComplete()) {
            break;

            i++;
        }
    }
    return indices;
}


void Renderer::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        int score = RateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        m_physicalDevice = candidates.rbegin()->second;
    }
    else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

}

void Renderer::AddToRenderQueue(const unsigned int RenderPass, const Vector3& Pos, const unsigned int ModelID)
{

    glm::mat4x4 transform(1);

    transform = glm::translate(transform, glm::vec3(Pos.x, Pos.y, Pos.z));

    
    
    if (m_renderingTargets.at(RenderPass).m_mainObjectRenderQueue.find(ModelID) == m_renderingTargets.at(RenderPass).m_mainObjectRenderQueue.end())
    {
        m_renderingTargets.at(RenderPass).m_mainObjectRenderQueue.insert({ ModelID, {} });
    }
    
    
    m_renderingTargets.at(RenderPass).m_mainObjectRenderQueue.at(ModelID).m_subQueue[0].emplace(transform);
}

glm::mat4 Renderer::BuildMatrix(const Vector3& Position, const Vector3& Rotation)
{
    glm::mat4 matrix = glm::mat4x4(1);

    matrix = glm::translate(matrix, glm::vec3(Position.x, Position.y, Position.z));

    matrix = glm::rotate(matrix, glm::radians(Rotation.x), glm::vec3(1, 0, 0));

    matrix = glm::rotate(matrix, glm::radians(Rotation.y), glm::vec3(0, 1, 0));

    matrix = glm::rotate(matrix, glm::radians(Rotation.z), glm::vec3(0, 0, 1));

    return matrix;
}

void Renderer::PositionCamera(const Vector3& Position, const Vector3& Rotation)
{
    m_camMatrix = BuildMatrix(Position, Rotation);
    /*

    m_camMatrix = glm::mat4x4(1);

    m_camMatrix = glm::translate(m_camMatrix, glm::vec3(Position.x, Position.y, Position.z));
    
    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.x), glm::vec3(1, 0, 0));

    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.y), glm::vec3(0, 1, 0));

    m_camMatrix = glm::rotate(m_camMatrix, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
    */
    
}

void Renderer::PositionLight(const Vector3& Position, const Vector3& Rotation)
{
    m_lightMatrix = BuildMatrix(Position, Rotation);
}

void GraphicsRenderPass::Init(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const std::vector<VkImageView>& ShadowMapImageView, const VkImageView& ShadowDepthImageView, const VkImageView DepthImageView, const VkSurfaceKHR& Surface, const GraphicsRenderPassOptions Options)
{
    if (Options.IsShadowMapper)
    {
        m_isShadowMapping = true;
    }

    CreateRenderPass(PhysicalDevice, Device, SwapChainImageFormat, Options.LoadOp, Options.StoreOp);
    CreateDescriptorSetLayout(Device);
    CreateDescriptorSet(Device);
    CreateUniformBuffer(PhysicalDevice, Device);
    CreateGraphicsPipeline(Device, SwapChainExtent, Options.VertexShader, Options.FragmentShader, Options.Topology);

    CreateFrameBuffers(Device, ImageViews, ShadowMapImageView, ShadowDepthImageView, DepthImageView, SwapChainExtent);
    CreateCommandPool(PhysicalDevice, Device, Surface);
    CreateCommandBuffer(Device);
}

void GraphicsRenderPass::CreateRenderPass(const VkPhysicalDevice PhysicalDevice, const VkDevice& Device, const VkFormat& SwapChainImageFormat, const VkAttachmentLoadOp& LoadOp, const VkAttachmentStoreOp& StoreOp)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = LoadOp;
    colorAttachment.storeOp = StoreOp;

    colorAttachment.stencilLoadOp = LoadOp;
    colorAttachment.stencilStoreOp = StoreOp;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = Renderer::FindDepthFormat(PhysicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = LoadOp;
    depthAttachment.storeOp = StoreOp;
    depthAttachment.stencilLoadOp = LoadOp;
    depthAttachment.stencilStoreOp = StoreOp;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GraphicsShadowPass::Init(const VkPhysicalDevice PhysicalDevice, const VkDevice Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const VkImageView& ShadowDepthImageView, const VkSurfaceKHR& Surface, const GraphicsRenderPassOptions Options)
{
    CreateRenderPass(PhysicalDevice, Device, SwapChainImageFormat, Options.LoadOp, Options.StoreOp);
    CreateDescriptorSetLayout(Device);
    CreateDescriptorSet(Device);
    CreateUniformBuffer(PhysicalDevice, Device);
    CreateGraphicsPipeline(Device, SwapChainExtent, Options.VertexShader, Options.FragmentShader, Options.Topology);

    CreateFrameBuffers(Device, ImageViews, ShadowDepthImageView, SwapChainExtent);
    CreateCommandPool(PhysicalDevice, Device, Surface);
    CreateCommandBuffer(Device);
}

void GraphicsShadowPass::CreateRenderPass(const VkPhysicalDevice PhysicalDevice, const VkDevice& Device, const VkFormat& SwapChainImageFormat, const VkAttachmentLoadOp& LoadOp, const VkAttachmentStoreOp StoreOp)
{
    /*
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = LoadOp;
    colorAttachment.storeOp = StoreOp;

    colorAttachment.stencilLoadOp = LoadOp;
    colorAttachment.stencilStoreOp = StoreOp;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    */
    /*
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    */

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = Renderer::FindDepthFormat(PhysicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = LoadOp;
    depthAttachment.storeOp = StoreOp;
    depthAttachment.stencilLoadOp = LoadOp;
    depthAttachment.stencilStoreOp = StoreOp;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 0;
    //subpass.pColorAttachments = &colorAttachmentRef;

    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { /*colorAttachment, */depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GraphicsShadowPass::CreateDescriptorSetLayout(const VkDevice& Device)
{
    std::array<VkDescriptorSetLayoutBinding, 1> uboBindings{};

    uboBindings[0].binding = 0;
    uboBindings[0].descriptorCount = 1;
    uboBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    /*
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBinding.descriptorCount = 5;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
    */

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = uboBindings.data();

    if (vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

}

void GraphicsShadowPass::CreateDescriptorSet(const VkDevice& Device)
{
    VkDescriptorPoolSize type_count[1];
    type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_count[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptor_pool{};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = 2;
    descriptor_pool.pPoolSizes = type_count;

    if (vkCreateDescriptorPool(Device, &descriptor_pool, NULL, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pool");
    }

    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.descriptorPool = m_descriptorPool;
    createInfo.descriptorSetCount = 1;
    createInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(Device, &createInfo, &m_uniformDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set");
    }
}

void GraphicsShadowPass::CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device)
{
    VkDeviceSize bufferSize = sizeof(ShadowUniformBufferObject);

    m_uniformBuffersMapped.resize(sizeof(ShadowUniformBufferObject));

    Renderer::CreateBuffer(PhysicalDevice, Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);
    vkMapMemory(Device, m_uniformBufferMemory, 0, bufferSize, 0, &m_uniformBuffersMapped[0]);
}

void GraphicsShadowPass::CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent, const std::string& VertexShader, const std::string& FragmentShader, const VkPrimitiveTopology& Topology)
{
    auto vertShaderCode = ReadFile(VertexShader.c_str());
    auto fragShaderCode = ReadFile(FragmentShader.c_str());

    m_vertexShaderModule = NewCreateShaderModule(Device, vertShaderCode);
    m_fragmentShaderModule = NewCreateShaderModule(Device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = m_vertexShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = Topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SwapChainExtent.width;
    viewport.height = (float)SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstant);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

    if (vkCreatePipelineLayout(Device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;

    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(Device, m_vertexShaderModule, nullptr);
    vkDestroyShaderModule(Device, m_fragmentShaderModule, nullptr);
}

void GraphicsShadowPass::CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const VkImageView& ShadowDepthImageView, const VkExtent2D& SwapChainExtent)
{
    m_frameBuffers.resize(ImageViews.size());

    for (size_t i = 0; i < ImageViews.size(); i++) {
        /*
        VkImageView attachments[] = {
            ImageViews[i],
            DepthImageView
        };
        */

        std::array<VkImageView, 1> attachments = {
            //ImageViews[i],
            ShadowDepthImageView
        };


        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = SwapChainExtent.width;
        framebufferInfo.height = SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Device, &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void GraphicsShadowPass::CreateCommandPool(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface)
{
    QueueFamilyIndices queueFamilyIndices = Renderer::FindQueueFamilies(PhysicalDevice, Surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(Device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void GraphicsShadowPass::CreateCommandBuffer(const VkDevice& Device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void GraphicsShadowPass::NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const ShadowUniformBufferObject UniformBuffer, const std::array<VkSampler, gImageBufferAmount>& Textures, const std::array<VkImageView, gImageBufferAmount>& ImageViews, const VkImageView& ShadowMapImageView)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_frameBuffers[ImageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = SwapChainExtent;
    /*
    std::array<VkClearValue, 2> clearValues{};

    clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.f} };
    clearValues[1].depthStencil = { 1.f, 0 };
    */
    std::array<VkClearValue, 1> clearValues{};

    clearValues[0].depthStencil = { 1.f, 0 };

    ////renderPassInfo.clearValueCount = 1;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    /*
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = Texture;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ImageView;
    */

    memcpy(m_uniformBuffersMapped[0], &UniformBuffer, sizeof(ShadowUniformBufferObject));

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ShadowUniformBufferObject);

    std::array<VkWriteDescriptorSet, 1> writeInfos{};
    writeInfos[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfos[0].descriptorCount = 1;
    writeInfos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfos[0].dstBinding = 0;
    writeInfos[0].dstSet = m_uniformDescriptorSet;
    writeInfos[0].pBufferInfo = &bufferInfo;


    vkUpdateDescriptorSets(Device, 1, writeInfos.data(), 0, nullptr);

    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_uniformDescriptorSet, 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(SwapChainExtent.width);
    viewport.height = static_cast<float>(SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SwapChainExtent;
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void GraphicsShadowPass::NewOnRendorObjBegin(const VkBuffer& VertexBuffer, const VkBuffer& IndexBuffer)
{
    VkBuffer vertexBuffers[] = { VertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(m_commandBuffer, IndexBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT16);
}

void GraphicsShadowPass::NewOnRendorObj(const int VertAmount, const PushConstant& Uniform, const int IndexAmount)
{
    //memcpy(m_uniformBuffersMapped[0], &Uniform, sizeof(Uniform));

    PushConstant unique(Uniform);

    vkCmdPushConstants(m_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &unique);

    //vkCmdDraw(m_commandBuffer, static_cast<uint32_t>(VertAmount), 1, 0, 0);
    vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(IndexAmount), 1, 0, 0, 0);
}

void GraphicsShadowPass::NewOnRenderFinish()
{
    vkCmdEndRenderPass(m_commandBuffer);

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void GraphicsRenderPass::CreateDescriptorSetLayout(const VkDevice& Device)
{
    /*
    if (m_isShadowMapping)
        return;
    */
    std::array<VkDescriptorSetLayoutBinding, 3> uboBindings{};

    uboBindings[0].binding = 0;
    uboBindings[0].descriptorCount = 1;
    uboBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    uboBindings[1].binding = 1;
    uboBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboBindings[1].descriptorCount = gImageBufferAmount;


    uboBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    uboBindings[1].pImmutableSamplers = nullptr; // Optional

    uboBindings[2].binding = 2;
    uboBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboBindings[2].descriptorCount = 1;
    uboBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    uboBindings[2].pImmutableSamplers = nullptr; // Optional

    /*
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBinding.descriptorCount = 5;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
    */

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = uboBindings.data();

    if (vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }


}

void GraphicsRenderPass::CreateComputeDescriptorSetLayout(const VkDevice& Device)
{
    /*
    if (!m_isShadowMapping)
        return;
    */
}

void GraphicsRenderPass::CreateDescriptorSet(const VkDevice& Device)
{
    /*
    if (m_isShadowMapping)
        return;
    */

    VkDescriptorPoolSize type_count[3];
    type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_count[0].descriptorCount = 1;

    type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    type_count[1].descriptorCount = gImageBufferAmount;

    type_count[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    type_count[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptor_pool{};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = 2;
    descriptor_pool.pPoolSizes = type_count;

    if (vkCreateDescriptorPool(Device, &descriptor_pool, NULL, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pool");
    }

    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.descriptorPool = m_descriptorPool;
    createInfo.descriptorSetCount = 1;
    createInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(Device, &createInfo, &m_uniformDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set");
    }
}

void GraphicsRenderPass::CreateComputeDescriptorSet(const VkDevice& Device)
{
    /*
    if (!m_isShadowMapping)
        return;
    */
}

void GraphicsRenderPass::CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject) + sizeof(UniformTextureBuffer);

    m_uniformBuffersMapped.resize(sizeof(UniformTextureBuffer) + sizeof(UniformBufferObject));

    Renderer::CreateBuffer(PhysicalDevice, Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);
    vkMapMemory(Device, m_uniformBufferMemory, 0, bufferSize, 0, &m_uniformBuffersMapped[0]);
}



void GraphicsRenderPass::CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent, const std::string& VertexShader, const std::string& FragmentShader, const VkPrimitiveTopology& Topology)
{
    auto vertShaderCode = ReadFile(VertexShader.c_str());
    auto fragShaderCode = ReadFile(FragmentShader.c_str());

    m_vertexShaderModule = CreateShaderModule(Device, vertShaderCode);
    m_fragmentShaderModule = CreateShaderModule(Device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = m_vertexShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = Topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SwapChainExtent.width;
    viewport.height = (float)SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstant);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

    if (vkCreatePipelineLayout(Device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;

    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(Device, m_vertexShaderModule, nullptr);
    vkDestroyShaderModule(Device, m_fragmentShaderModule, nullptr);
}

void GraphicsRenderPass::CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const std::vector<VkImageView>& ShadowMapImageView, const VkImageView& ShadowDepthImageView, const VkImageView DepthImageView, const VkExtent2D& SwapChainExtent)
{
    m_frameBuffers.resize(ImageViews.size());

    for (size_t i = 0; i < ImageViews.size(); i++) {
        /*
        VkImageView attachments[] = {
            ImageViews[i],
            DepthImageView
        };
        */

        std::array<VkImageView, 2> attachments = {
            ImageViews[i],
            DepthImageView
        };

        if (m_isShadowMapping)
        {
            attachments[0] = ShadowMapImageView[i];
            attachments[1] = ShadowDepthImageView;
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = SwapChainExtent.width;
        framebufferInfo.height = SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Device, &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void GraphicsRenderPass::CreateCommandPool(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkSurfaceKHR& Surface)
{
    QueueFamilyIndices queueFamilyIndices = Renderer::FindQueueFamilies(PhysicalDevice, Surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

    if (vkCreateCommandPool(Device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void GraphicsRenderPass::CreateCommandBuffer(const VkDevice& Device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void GraphicsRenderPass::RecordCommandBuffer(const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_frameBuffers[ImageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};

    clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.f}};
    clearValues[1].depthStencil = { 1.f, 0 };

    if (m_isShadowMapping)
    {
        clearValues[0].color = { {1,1,1,1} };
    }

    ////renderPassInfo.clearValueCount = 1;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(SwapChainExtent.width);
    viewport.height = static_cast<float>(SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SwapChainExtent;
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_commandBuffer);

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkShaderModule GraphicsRenderPass::CreateShaderModule(const VkDevice& Device, const std::vector<char>& Code)
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

void GraphicsRenderPass::NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const UniformBufferObject UniformBuffer, const std::array<VkSampler, gImageBufferAmount>& Textures, const std::array<VkImageView, gImageBufferAmount>& ImageViews, const VkImageView& ShadowMapImageView)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_frameBuffers[ImageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};

    clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.f} };
    clearValues[1].depthStencil = { 1.f, 0 };

    if (m_isShadowMapping)
    {
        clearValues[0].color = { {1,1,1,1} };
    }

    ////renderPassInfo.clearValueCount = 1;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
      
    std::array<VkDescriptorImageInfo, gImageBufferAmount> imageInfos{};
    for (int i = 0; i < gImageBufferAmount; i++)
    {
        imageInfos[i].sampler = Textures[i];
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = ImageViews[i];
    }
    /*
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = Texture;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ImageView;
    */

    memcpy(m_uniformBuffersMapped[0], &UniformBuffer, sizeof(UniformBufferObject));

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> writeInfos{};
    writeInfos[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfos[0].descriptorCount = 1;
    writeInfos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfos[0].dstBinding = 0;
    writeInfos[0].dstSet = m_uniformDescriptorSet;
    writeInfos[0].pBufferInfo = &bufferInfo;


    //VkWriteDescriptorSet writeInfo{};

    writeInfos[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfos[1].descriptorCount = 5;
    writeInfos[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfos[1].dstBinding = 1;
    writeInfos[1].dstSet = m_uniformDescriptorSet;
    writeInfos[1].pImageInfo = imageInfos.data();

    VkDescriptorImageInfo shadowMapInfo;

    shadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadowMapInfo.sampler = Textures[0];
    shadowMapInfo.imageView = ShadowMapImageView;

    writeInfos[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfos[2].descriptorCount = 1;
    writeInfos[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfos[2].dstBinding = 2;
    writeInfos[2].dstSet = m_uniformDescriptorSet;
    writeInfos[2].pImageInfo = &shadowMapInfo;


    vkUpdateDescriptorSets(Device, 3, writeInfos.data(), 0, nullptr);

    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_uniformDescriptorSet, 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(SwapChainExtent.width);
    viewport.height = static_cast<float>(SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SwapChainExtent;
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void GraphicsRenderPass::NewOnRendorObjBegin(const VkBuffer& VertexBuffer, const VkBuffer& IndexBuffer)
{
    VkBuffer vertexBuffers[] = { VertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(m_commandBuffer, IndexBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT16);
}

void GraphicsRenderPass::NewOnRendorObj(const int VertAmount, const PushConstant& Uniform, const int IndexAmount)
{
    //memcpy(m_uniformBuffersMapped[0], &Uniform, sizeof(Uniform));

    PushConstant unique(Uniform);
    
    vkCmdPushConstants(m_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &unique);

    //vkCmdDraw(m_commandBuffer, static_cast<uint32_t>(VertAmount), 1, 0, 0);
    vkCmdDrawIndexed(m_commandBuffer, static_cast<uint32_t>(IndexAmount), 1, 0, 0, 0);
}

void GraphicsRenderPass::NewOnRenderFinish()
{
    vkCmdEndRenderPass(m_commandBuffer);

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void GraphicsRenderPass::CleanUp(const vk::Device& Device)
{
    /*
    for (auto& it : m_modelBuffers)
    {
        it.clear(Device);
    }
    */
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

    //m_camMatrix = m_lightMatrix;

    //bring back
    OnGUI();

    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device, 1, &m_inFlightFence);

    uint32_t imageIndex;

    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    //vkResetCommandBuffer(m_renderPasses[0].m_commandBuffer, 0);

    for (auto& renderPass : m_renderPasses)
    {
        vkResetCommandBuffer(renderPass.second.m_commandBuffer, 0);
    }

    for (auto& shadowPass : m_shadowPasses)
    {
        vkResetCommandBuffer(shadowPass.second.m_commandBuffer, 0);
    }

    vkResetCommandBuffer(m_guiPass.m_commandBuffer, 0);

    //m_renderPasses[0].RecordCommandBuffer(imageIndex, m_swapChainExtent);

    ShadowUniformBufferObject shadowUniform{};
    shadowUniform.m_shadowModelViewProj = GetViewProjectionMatrix(m_extents, m_lightMatrix);
    
    
    //shadow map terrain
    m_shadowPasses[0].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, shadowUniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);
    
    for (int i = 0; i < m_terrain.at(0).m_size; i++)
    {

        m_shadowPasses[0].NewOnRendorObjBegin(m_terrain.at(0).m_TerrainModel.m_vertexBuffer, m_terrain.at(0).m_sectionIndices[i].m_indexBuffer);

        glm::mat4 ViewProjection = GetViewProjectionMatrix(m_extents, m_camMatrix);

        PushConstant push{};
        push.m_transform = 
            glm::mat4(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1);
        push.m_usesTexture = true;

        m_shadowPasses[0].NewOnRendorObj(m_terrain.at(0).m_TerrainModel.m_vertices.size(), push, m_terrain.at(0).m_sectionIndices[i].m_elements.size());

    }
    
    m_shadowPasses[0].NewOnRenderFinish();
    

    //shadow map trees
    m_shadowPasses[1].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, shadowUniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);

    for (int i = 0; i < m_terrain.at(0).m_treePositions.size(); i++)
    {
        glm::vec3 LtoT = m_terrain.at(0).m_treePositions.at(i) - glm::vec3(0, 20, 0);

        if (glm::dot(glm::normalize(LtoT), glm::vec3(0, 0, 1)) < 0.75)
        {
            continue;
        }

        m_shadowPasses.at(1).NewOnRendorObjBegin(m_modelDatas.at(3).SubModels.at(0).m_vertexBuffer, m_modelDatas.at(3).SubModels.at(0).m_indexBuffer);
        PushConstant push{};
        glm::mat4 model = glm::mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        model = glm::translate(model, m_terrain.at(0).m_treePositions[i]);
        //push.m_transform = ViewProjection * model;
        push.m_transform = model;
        push.m_usesTexture = true;
        m_shadowPasses.at(1).NewOnRendorObj(m_modelDatas.at(3).SubModels.at(0).m_vertices.size(), push, m_modelDatas.at(3).SubModels.at(0).m_elements.size());
    }
   
    m_shadowPasses.at(1).NewOnRenderFinish();

    //iterate passes
    for (auto& ShadowPass : m_shadowTargets)
    {
        if (ShadowPass.second.m_mainObjectRenderQueue.empty())
            continue;
        
        m_shadowPasses.at(ShadowPass.first).NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, shadowUniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);
        //iterate models
        for (auto& ShadowSelection : ShadowPass.second.m_mainObjectRenderQueue)
        {
            // nothing to render or main model doesn't exist
            if (ShadowSelection.second.m_subQueue.empty() || m_modelDatas.find(ShadowSelection.first) == m_modelDatas.end())
                continue;

            //iterate render sub models
            for (auto& ShadowSubSelection : ShadowSelection.second.m_subQueue)
            {
                unsigned int subModel = 0;

                for (unsigned int i = 0x1; i < 10; i = i << 1)
                {
                    //check if is valid selection
                    if ((ShadowSubSelection.first & i) == 0)
                    {
                        subModel++;
                        continue;
                    }

                    //check if model exists
                    if (m_modelDatas.at(ShadowSelection.first).SubModels.find(subModel - 1) == m_modelDatas.at(ShadowSelection.first).SubModels.end())
                        continue;

                    m_renderPasses.at(ShadowPass.first).NewOnRendorObjBegin(m_modelDatas.at(ShadowSelection.first).SubModels.at(subModel - 1).m_vertexBuffer, m_modelDatas.at(ShadowSelection.first).SubModels.at(subModel - 1).m_indexBuffer);

                    //iterate positions
                    while (!ShadowSubSelection.second.empty())
                    {
                        PushConstant push{};
                        push.m_transform = ShadowSubSelection.second.front();
                        push.m_usesTexture = true;

                        m_renderPasses.at(ShadowPass.first).NewOnRendorObj(
                            m_modelDatas.at(ShadowSelection.first).SubModels.at(subModel - 1).m_vertices.size(),
                            push,
                            m_modelDatas.at(ShadowSelection.first).SubModels.at(subModel - 1).m_elements.size());
                        ShadowSubSelection.second.pop();
                    }
                    subModel++;
                }
            }
        }
    }
    
    
    
    UniformBufferObject uniform{};
    uniform.m_projectionMatrix = GetProjectionMatrix(m_extents);
    uniform.m_veiwMatrix = GetViewMatrix(m_camMatrix);
    uniform.m_lightViewMatrix = GetViewMatrix(m_lightMatrix);

    //render map terrain
    m_renderPasses[0].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, uniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);

    for (int i = 0; i < m_terrain.at(0).m_size; i++)
    {

        m_renderPasses[0].NewOnRendorObjBegin(m_terrain.at(0).m_TerrainModel.m_vertexBuffer, m_terrain.at(0).m_sectionIndices[i].m_indexBuffer);

        glm::mat4 ViewProjection = GetViewProjectionMatrix(m_extents, m_camMatrix);

        PushConstant push{};
        push.m_transform =
            glm::mat4(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1);
        push.m_usesTexture = true;

        m_renderPasses[0].NewOnRendorObj(m_terrain.at(0).m_TerrainModel.m_vertices.size(), push, m_terrain.at(0).m_sectionIndices[i].m_elements.size());

    }

    m_renderPasses[0].NewOnRenderFinish();


    //render map trees
    m_renderPasses[1].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, uniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);

    for (int i = 0; i < m_terrain.at(0).m_treePositions.size(); i++)
    {
        glm::vec3 LtoT = m_terrain.at(0).m_treePositions.at(i) - glm::vec3(0, 20, 0);

        if (glm::dot(glm::normalize(LtoT), glm::vec3(0, 0, 1)) < 0.75)
        {
            continue;
        }

        m_renderPasses.at(1).NewOnRendorObjBegin(m_modelDatas.at(3).SubModels.at(0).m_vertexBuffer, m_modelDatas.at(3).SubModels.at(0).m_indexBuffer);
        PushConstant push{};
        glm::mat4 model = glm::mat4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        model = glm::translate(model, m_terrain.at(0).m_treePositions[i]);
        //push.m_transform = ViewProjection * model;
        push.m_transform = model;
        push.m_usesTexture = true;
        m_renderPasses.at(1).NewOnRendorObj(m_modelDatas.at(3).SubModels.at(0).m_vertices.size(), push, m_modelDatas.at(3).SubModels.at(0).m_elements.size());
    }

    m_renderPasses.at(1).NewOnRenderFinish();


    /*
    //iterate render passes
    for (auto& RenderPass : m_renderingTargets)
    {
        if (RenderPass.second.m_mainObjectRenderQueue.empty() && RenderPass.first != 2)
            continue;

        UniformBufferObject uniform1{};
        uniform1.m_projectionMatrix = GetProjectionMatrix(m_extents);
        uniform1.m_veiwMatrix = GetViewMatrix(m_camMatrix);
        uniform1.m_lightViewMatrix = GetViewMatrix(m_lightMatrix);

        m_renderPasses[RenderPass.first].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, uniform1, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);

        //glm::mat4 ViewProjection = m_renderPasses.at(RenderPass.first).GetViewProjectionMatrix(m_extents, m_camMatrix);

        // iterate render models
        for (auto& RenderSelection : RenderPass.second.m_mainObjectRenderQueue)
        {
            if (RenderSelection.second.m_subQueue.empty() || m_modelDatas.find(RenderSelection.first) == m_modelDatas.end())
                continue;

            //iterate render sub models
            for (auto& RenderSubSelection : RenderSelection.second.m_subQueue)
            {
                unsigned int subModel = 0;

                for (unsigned int i = 0x1; i < 10; i = i << 1)
                {
                    //check if is valid selection
                    if ((RenderSubSelection.first & i) == 0)
                    {
                        subModel++;
                        continue;
                    }

                    //check if model exists
                    if (m_modelDatas.at(RenderSelection.first).SubModels.find(subModel-1) == m_modelDatas.at(RenderSelection.first).SubModels.end())
                        continue;
                    
                    m_renderPasses.at(RenderPass.first).NewOnRendorObjBegin(m_modelDatas.at(RenderSelection.first).SubModels.at(subModel-1).m_vertexBuffer, m_modelDatas.at(RenderSelection.first).SubModels.at(subModel-1).m_indexBuffer);

                    //iterate positions
                    while (!RenderSubSelection.second.empty())
                    {
                        PushConstant push{};
                        push.m_transform = RenderSubSelection.second.front();
                        push.m_usesTexture = true;

                        m_renderPasses.at(RenderPass.first).NewOnRendorObj(
                            m_modelDatas.at(RenderSelection.first).SubModels.at(subModel-1).m_vertices.size(),
                            push,
                            m_modelDatas.at(RenderSelection.first).SubModels.at(subModel-1).m_elements.size());
                        RenderSubSelection.second.pop();
                    }
                    subModel++;
                }
            }
        }
        //draw trees
        if (RenderPass.first == 3 || RenderPass.first == 1)
        {
            for (int i = 0; i < m_terrain.at(0).m_treePositions.size(); i++)
            {
                
                glm::vec3 LtoT = m_terrain.at(0).m_treePositions.at(i) - glm::vec3(0, 20, 0);

                if (glm::dot(glm::normalize(LtoT), glm::vec3(0, 0, 1)) < 0.75)
                {
                    continue;
                }

                m_renderPasses[RenderPass.first].NewOnRendorObjBegin(m_modelDatas.at(3).SubModels.at(0).m_vertexBuffer, m_modelDatas.at(3).SubModels.at(0).m_indexBuffer);
                PushConstant push{};
                glm::mat4 model = glm::mat4(
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
                model = glm::translate(model, m_terrain.at(0).m_treePositions[i]);
                //push.m_transform = ViewProjection * model;
                push.m_transform = model;
                push.m_usesTexture = true;
                m_renderPasses[RenderPass.first].NewOnRendorObj(m_modelDatas.at(3).SubModels.at(0).m_vertices.size(), push, m_modelDatas.at(3).SubModels.at(0).m_elements.size());
            }
            if (RenderPass.first == 3)
            {
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_renderPasses.at(3).m_commandBuffer);
            }
        }
        else if (RenderPass.first == 2)
        {
            //normal render terrain
            //m_renderPasses[2].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, uniform, m_samplers, m_imageVeiws, m_shadowMapImageViews.at(imageIndex));

            for (int i = 0; i < m_terrain.at(0).m_size; i++)
            {

                m_renderPasses[2].NewOnRendorObjBegin(m_terrain.at(0).m_TerrainModel.m_vertexBuffer, m_terrain.at(0).m_sectionIndices[i].m_indexBuffer);

                glm::mat4 ViewProjection = GetViewProjectionMatrix(m_extents, m_camMatrix);

                PushConstant push{};
                push.m_transform =
                    glm::mat4(
                        1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
                push.m_usesTexture = true;


                m_renderPasses[2].NewOnRendorObj(m_terrain.at(0).m_TerrainModel.m_vertices.size(), push, m_terrain.at(0).m_sectionIndices[i].m_elements.size());

            }
            //m_renderPasses[2].NewOnRenderFinish();
        }

        m_renderPasses.at(RenderPass.first).NewOnRenderFinish();
    }
   
    */
    
    m_guiPass.NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, uniform, m_samplers, m_imageVeiws, m_shadowMapDepthImageView);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_guiPass.m_commandBuffer);

    m_guiPass.NewOnRenderFinish();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

   // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer commandBuffers[] = { m_shadowPasses.at(0).m_commandBuffer, m_shadowPasses.at(1).m_commandBuffer,
        m_shadowPasses.at(2).m_commandBuffer, m_renderPasses.at(0).m_commandBuffer, m_renderPasses.at(1).m_commandBuffer,
        m_renderPasses.at(2).m_commandBuffer, m_guiPass.m_commandBuffer};

    submitInfo.commandBufferCount = 7;
    //submitInfo.pCommandBuffers = &m_renderPasses[0].m_commandBuffer;
    submitInfo.pCommandBuffers = commandBuffers;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    VkResult res;

    res = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence);

    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);

    

    //renderPassInfo.dependencyCount = 1;
    //renderPassInfo.pDependencies = &dependency;

    

    //will bring back in
    /*
    for (auto& RenderTarget : m_renderingTargets)
    {
        for (auto& ModelAndTransforms : RenderTarget.second)
        {

            int lastUsed = -1;
            int Current = 0;

            vk::ResultValue<uint32_t> CurrentBuffer = m_renderPasses.at(RenderTarget.first).OnRenderStart(m_device, m_swapChainData, m_commandBuffer, m_surfaceData);

            glm::mat4 viewProjection = m_renderPasses.at(RenderTarget.first).GetViewProjectionMatrix(m_surfaceData, m_camMatrix);
            
            //m_commandBuffer.setViewport(
                0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_surfaceData.extent.width), static_cast<float>(m_surfaceData.extent.height), 0.0f, 1.0f));

            //m_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_surfaceData.extent));
            

            while (ModelAndTransforms.second.size() > 0)
            {
                glm::mat4x4 Transform = ModelAndTransforms.second.front();
                
                ModelAndTransforms.second.pop();

                m_renderPasses.at(RenderTarget.first).SetUniformDataModelViewProjection(viewProjection, m_surfaceData, m_physicalDevice, m_device, Transform, m_camMatrix, m_shouldUpdateDescriptor);

                m_renderPasses.at(RenderTarget.first).OnRenderObj(m_commandBuffer, m_modelDatas.at(ModelAndTransforms.first).m_vertexBuffer, m_modelDatas.at(ModelAndTransforms.first).m_vertices, CurrentBuffer, m_surfaceData, m_modelDatas.at(ModelAndTransforms.first).m_vertices.size());
            }
        
            m_renderPasses.at(RenderTarget.first).OnRenderFinish(CurrentBuffer, m_commandBuffer, m_device, m_swapChainData, m_graphicsQueue, m_presentQueue);
        }
    m_shouldUpdateDescriptor = false;
    }
    */
    
}

bool Renderer::WindowShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Renderer::PollEvents()
{
    glfwPollEvents();
}

void Renderer::OnGUI()
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
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    //ImGui::GetIO().DisplaySize = ImVec2(100, 100);
    ImGui::NewFrame();

    ImGui::Begin("Viewport");
    ImGui::Image((ImTextureID)m_viewportDescriptorSet, ImVec2(m_extents.width, m_extents.height));
    ImGui::End();

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
        (float*)&GetPhysicsInstance().m_downVec)) {
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

                unsigned int offset = sizeof(char) * 8;

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
}

uint32_t Renderer::FindMemoryType(const VkPhysicalDevice& PhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
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
}

void Renderer::CreateBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(PhysicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(Device, buffer, bufferMemory, 0);
}

bool Renderer::HasStencilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat Renderer::FindSupportedFormat(const VkPhysicalDevice PhysicalDevice, const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
    for (VkFormat format : Candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &props);
        if (Tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & Features) == Features) {
            return format;
        }
        else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & Features) == Features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

VkFormat Renderer::FindDepthFormat(const VkPhysicalDevice PhysicalDevice)
{
    return FindSupportedFormat(
        PhysicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void Renderer::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat(m_physicalDevice);

    CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    
    depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("grass.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(m_physicalDevice, m_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free(pixels);

    m_imageDatas.emplace(0, nullptr);
    m_imageMemory.emplace(0, nullptr);

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_imageDatas[0], m_imageMemory[0]);

    TransitionImageLayout(m_imageDatas[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, m_imageDatas[0], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(m_imageDatas[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Renderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image, imageMemory, 0);
}

VkCommandBuffer Renderer::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Renderer::EndSingleTimeCommands(VkCommandBuffer CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CommandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_pool, 1, &CommandBuffer);
}

void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = OldLayout;
    barrier.newLayout = NewLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = Image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(Format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        Width,
        Height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        Buffer,
        Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CreateTextureImageView()
{
    for (int i = 0; i < 5; i++)
    {
        m_imageVeiws[i] = CreateImageView(m_imageDatas[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    //m_imageVeiw = CreateImageView(m_imageDatas[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView Renderer::CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = Format;
    viewInfo.subresourceRange.aspectMask = AspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Renderer::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;


    for (size_t i = 0; i < 5; i++)
    {
        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_samplers[i]) != VK_SUCCESS) {
             throw std::runtime_error("failed to create texture sampler!");
        }
    }
}

void Renderer::CreateInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto Extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
    createInfo.ppEnabledExtensionNames = Extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_vulkanInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

Renderer::~Renderer()
{
    /* VULKAN_KEY_END */

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_vulkanInstance, debugMessenger, nullptr);
    }

    for (auto& it : m_modelDatas)
    {
        for (auto& it2 : it.second.SubModels)
        {

        //it.second.m_vertexBufferData.clear(m_device);
        vkDestroyBuffer(m_device, it2.second.m_vertexBuffer, nullptr);
        vkFreeMemory(m_device, it2.second.m_vertexBufferMemory, nullptr);
        }
    }

    for (auto imageView : m_swapChainImageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    for (auto& it : m_imageDatas)
    {
        vkDestroyImage(m_device, it.second, nullptr);
    }
    for (auto& it : m_imageMemory)
    {
        vkFreeMemory(m_device, it.second, nullptr);
    }

    //vkDestroySampler(m_device, m_sampler, nullptr);
    //vkDestroyImageView(m_device, m_imageVeiw, nullptr);

    for (auto& sampler : m_samplers)
    {
        vkDestroySampler(m_device, sampler, nullptr);
    }

    for (auto& TextureView : m_imageVeiws)
    {
        vkDestroyImageView(m_device, TextureView, nullptr);
    }

    for (auto& it : m_renderPasses)
    {
        vkResetDescriptorPool(m_device, it.second.m_descriptorPool, {});
        vkDestroyDescriptorPool(m_device, it.second.m_descriptorPool, nullptr);
        vkDestroyPipeline(m_device, it.second.m_pipeline, nullptr);
        vkDestroyPipelineCache(m_device, it.second.m_pipelineCache, nullptr);
        vkDestroyPipelineLayout(m_device, it.second.m_pipelineLayout, nullptr);
        vkDestroyCommandPool(m_device, it.second.m_commandPool, nullptr);
        it.second.CleanUp(m_device);
        

        for (auto& framebuffer : it.second.m_frameBuffers)
        {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
        vkDestroyShaderModule(m_device, it.second.m_vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_device, it.second.m_fragmentShaderModule, nullptr);
        vkDestroyRenderPass(m_device, it.second.m_renderPass, nullptr);
        //m_device.destroyDescriptorSetLayout(it.second.m_descriptorSetLayout);
        vkDestroyDescriptorSetLayout(m_device, it.second.m_descriptorSetLayout, nullptr);
    }

#if !defined( NDEBUG )
    //instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    glfwDestroyWindow(m_window);

    glfwTerminate();

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    vkDestroyCommandPool(m_device, m_pool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);

    vkDestroyInstance(m_vulkanInstance, nullptr);
    //m_vulkanInstance.destroy();
}
/*
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
    

}
*/

bool Mesh::LoadFromObj(const char* filename, std::vector<std::vector<Vertex>>& verts, std::vector<std::vector<uint16_t>>& elements)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./";

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::unordered_map<int, int> debugging{};

    //verts.emplace_back();
    //elements.emplace_back();
    verts.resize(shapes.size());
    elements.resize(shapes.size());

    //loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {

        size_t index_offset = 0;
        //loop over faces
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            //loop over vertices
            for (size_t v = 0; v < fv; v++)
            {
                Vertex newVertex{glm::vec4(), glm::vec4(), glm::vec2(), 0};

                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                newVertex.m_position = glm::vec4(vx, vy, vz, 1);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                    newVertex.m_uv = glm::vec2(tx, ty);
                }

                verts[s].emplace_back(newVertex);
            }
            index_offset += fv;

            // per face material
            //shapes[s].mesh.material_ids[f];
            //shapes.at(0).mesh.material_ids.at(f);
            debugging[shapes.at(s).mesh.material_ids.at(f)];
        }
    }

    return false;
}
