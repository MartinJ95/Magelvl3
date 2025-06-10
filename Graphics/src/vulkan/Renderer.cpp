#include "Renderer.h"
#include "ECS.h"
#include "Transform.h"
#include "PhysicsBase.h"
#include "imgui-1.91.8/misc/cpp/imgui_stdlib.h"
#include <map>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#define STB_IMAGE_IMPLEMENTATION
#include <stb-master/stb-master/stb_image.h>

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
    
    //mainWindowData.Swapchain = m_renderPasses.at(0).
    //mainWindowData.RenderPass = m_renderPasses.at(0).m_renderPass;
    //ImGui_ImplVulkanH_CreateOrResizeWindow(m_vulkanInstance, m_physicalDevice, m_device, &mainWindowData, m_graphicsAndPresentQueueFamilyIndex.first, nullptr, m_surfaceData.extent.width, m_surfaceData.extent.height, 2);
}

void Renderer::Init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);

    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();

    m_renderPasses.emplace(std::piecewise_construct, std::make_tuple(0), std::make_tuple());
    m_renderPasses[0].Init(m_physicalDevice, m_device, m_swapChainExtent, m_swapChainImageFormat, m_swapChainImageViews, m_surface);

    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();

    CreateSyncObjects();

    m_modelDatas.emplace(
        std::piecewise_construct, std::forward_as_tuple(0),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            GenerateBox(),
            std::vector<unsigned int>())

    );
    m_modelDatas.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(1),
        std::forward_as_tuple(m_physicalDevice,
            m_device,
            GenerateSphere(),
            std::vector<unsigned int>())

    );
    

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    ImGui_ImplVulkan_InitInfo info = {};
    info.Instance = m_vulkanInstance;
    info.PhysicalDevice = m_physicalDevice;
    info.Device = m_device;
    //info.DescriptorPool = m_renderPasses[0].m_descriptorPool;
    info.DescriptorPoolSize = 3;
    info.RenderPass = m_renderPasses[0].m_renderPass;
    info.MinImageCount = 3;
    info.ImageCount = 3;
    info.QueueFamily = m_graphicsAndPresentQueueFamilyIndex.first;
    info.Queue = m_graphicsQueue;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;


    //will bring back
    ImGui_ImplVulkan_Init(&info);

    for (auto& model : m_modelDatas)
    {
        for (int i = 0; i < model.second.m_vertices.size(); i = i + 3)
        {
            model.second.m_elements.emplace_back(i);
            model.second.m_elements.emplace_back(i + 1);
            model.second.m_elements.emplace_back(i + 2);
        }
        model.second.InitIndices(m_physicalDevice, m_device);
    }

    m_modelDatas.at(0).InitIndices(m_physicalDevice, m_device);
    m_modelDatas.at(1).InitIndices(m_physicalDevice, m_device);

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

void GraphicsRenderPass::Init(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device, const VkExtent2D& SwapChainExtent, const VkFormat& SwapChainImageFormat, const std::vector<VkImageView>& ImageViews, const VkSurfaceKHR& Surface)
{
    CreateRenderPass(Device, SwapChainImageFormat);
    CreateDescriptorSetLayout(Device);
    CreateDescriptorSet(Device);
    CreateUniformBuffer(PhysicalDevice, Device);
    CreateGraphicsPipeline(Device, SwapChainExtent);
    CreateFrameBuffers(Device, ImageViews, SwapChainExtent);
    CreateCommandPool(PhysicalDevice, Device, Surface);
    CreateCommandBuffer(Device);
}

void GraphicsRenderPass::CreateRenderPass(const VkDevice& Device, const VkFormat& SwapChainImageFormat)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void GraphicsRenderPass::CreateDescriptorSetLayout(const VkDevice& Device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }


}

void GraphicsRenderPass::CreateDescriptorSet(const VkDevice& Device)
{
    VkDescriptorPoolSize type_count[1];
    type_count[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    type_count[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptor_pool{};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = 1;
    descriptor_pool.pPoolSizes = type_count;

    if (vkCreateDescriptorPool(Device, &descriptor_pool, NULL, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pool");
    }

    VkDescriptorSetAllocateInfo createInfo{};
    createInfo.descriptorPool = m_descriptorPool;
    createInfo.descriptorSetCount = 1;
    createInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(Device, &createInfo, &m_uniformImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set");
    }
}

void GraphicsRenderPass::CreateUniformBuffer(const VkPhysicalDevice& PhysicalDevice, const VkDevice& Device)
{
    VkDeviceSize bufferSize = sizeof(UniformBuffer);

    m_uniformBuffersMapped.resize(sizeof(UniformBuffer));

    Renderer::CreateBuffer(PhysicalDevice, Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory);
    vkMapMemory(Device, m_uniformBufferMemory, 0, bufferSize, 0, &m_uniformBuffersMapped[0]);
}



void GraphicsRenderPass::CreateGraphicsPipeline(const VkDevice& Device, const VkExtent2D& SwapChainExtent)
{
    auto vertShaderCode = ReadFile("vert.spv");
    auto fragShaderCode = ReadFile("frag.spv");

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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

    if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(Device, m_vertexShaderModule, nullptr);
    vkDestroyShaderModule(Device, m_fragmentShaderModule, nullptr);
}

void GraphicsRenderPass::CreateFrameBuffers(const VkDevice& Device, const std::vector<VkImageView>& ImageViews, const VkExtent2D& SwapChainExtent)
{
    m_frameBuffers.resize(ImageViews.size());

    for (size_t i = 0; i < ImageViews.size(); i++) {
        VkImageView attachments[] = {
            ImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
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

    VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 0.1f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

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

glm::mat4 GraphicsRenderPass::GetViewProjectionMatrix(const VkExtent2D& SurfaceExtent, const glm::mat4& CamMatrix)
{
    glm::mat4x4 testMat = vk::su::createModelViewProjectionClipMatrix(SurfaceExtent);

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
    

    //vk::su::copyToDevice(Device, m_modelBuffers.at(currentBuffer).deviceMemory, m_mvpcMatrix);

    //vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_descriptorPool, m_descriptorSetLayout);

    //vk::su::updateDescriptorSets(Device, m_descriptorSet, { { vk::DescriptorType::eUniformBuffer, m_uniformBuffer.buffer, VK_WHOLE_SIZE, {} } }, {});
    //m_descriptorSets.emplace(Device.allocateDescriptorSets(descriptorSetAllocateInfo).front());
    //vk::su::updateDescriptorSets(Device, m_descriptorSets.top(), { { vk::DescriptorType::eUniformBuffer, m_modelBuffers.at(currentBuffer).buffer, VK_WHOLE_SIZE, {}}}, {});
    if (ShouldUpdate)
    {
        //vk::su::updateDescriptorSets(Device, m_descriptorSets.at(currentBuffer), {{vk::DescriptorType::eUniformBuffer, m_modelBuffers.at(currentBuffer).buffer, VK_WHOLE_SIZE, {}}}, {});
    }
}

void GraphicsRenderPass::OnRenderStart(const vk::Device &Device, vk::su::SwapChainData &SwapChainData, vk::CommandBuffer &CommandBuffer, vk::su::SurfaceData &SurfaceData)
{

    /*
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
    */
}

void GraphicsRenderPass::OnRenderObj(const vk::CommandBuffer& CommandBuffer, const VkBuffer& VertexBuffer, const std::vector<Vertex> VertData, const vk::ResultValue<uint32_t>& CurrentBuffer, const vk::su::SurfaceData& SurfaceData, const int VertexCount)
{
    /*
    CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSets.at(m_usedModelsAmount-1), nullptr);

    //CommandBuffer.bindVertexBuffers(0, Data.buffer,{0});

    VkBuffer vertexBuffers[] = { VertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(CommandBuffer, static_cast<uint32_t>(VertData.size()), 1, 0, 0);

    /*
    CommandBuffer.setViewport(
        0, vk::Viewport(0.0f, 0.0f, static_cast<float>(SurfaceData.extent.width), static_cast<float>(SurfaceData.extent.height), 0.0f, 1.0f));
    CommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), SurfaceData.extent));
    
    
    CommandBuffer.draw(VertexCount, 1, 0, 0);
    //CommandBuffer.draw(12 * 3, 1, 0, 0);
    */
}

void GraphicsRenderPass::OnRenderFinish(const vk::ResultValue<uint32_t> &CurrentBuffer, const vk::CommandBuffer& CommandBuffer, const vk::Device &Device, const vk::su::SwapChainData &SwapChainData, const vk::Queue &GraphicsQueue, const vk::Queue &PresentQueue)
{
 /*
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
    

    //need to allocate descriptors again after resetting
    //vkResetDescriptorPool(Device, m_descriptorPool, {});
 */
}

void GraphicsRenderPass::NewOnRenderStart(const VkDevice& Device, const uint32_t ImageIndex, const VkExtent2D& SwapChainExtent, const VkSampler& Texture, const VkImageView& ImageView)
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

    VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 0.1f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = Texture;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ImageView;

    VkWriteDescriptorSet writeInfo{};
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.dstBinding = 0;
    writeInfo.dstSet = m_uniformImage;
    writeInfo.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(Device, 1, &writeInfo, 0, nullptr);

    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_uniformImage, 0, nullptr);

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
    vkCmdBindIndexBuffer(m_commandBuffer, IndexBuffer, 0, VkIndexType::VK_INDEX_TYPE_MAX_ENUM);
}

void GraphicsRenderPass::NewOnRendorObj(const int VertAmount, const PushConstant& Uniform)
{
    memcpy(m_uniformBuffersMapped[0], &Uniform, sizeof(Uniform));

    PushConstant unique(Uniform);
    
    vkCmdPushConstants(m_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(UniformBuffer), &unique);

    vkCmdDraw(m_commandBuffer, static_cast<uint32_t>(VertAmount), 1, 0, 0);
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

    //bring back
    OnGUI();

    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device, 1, &m_inFlightFence);

    uint32_t imageIndex;

    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(m_renderPasses[0].m_commandBuffer, 0);

    //m_renderPasses[0].RecordCommandBuffer(imageIndex, m_swapChainExtent);

    for (auto& RenderPassTargets : m_renderingTargets)
    {
        m_renderPasses[RenderPassTargets.first].NewOnRenderStart(m_device, imageIndex, m_swapChainExtent, m_sampler, m_imageVeiw);

        for (auto& ModelTransforms : RenderPassTargets.second)
        {
            m_renderPasses[RenderPassTargets.first].NewOnRendorObjBegin(m_modelDatas.at(ModelTransforms.first).m_vertexBuffer, m_modelDatas.at(ModelTransforms.first).m_indexBuffer);
            while (!ModelTransforms.second.empty())
            {
                glm::mat4 ViewProjection = m_renderPasses.at(RenderPassTargets.first).GetViewProjectionMatrix(m_extents, m_camMatrix);
                PushConstant push{};
                push.m_transform = ViewProjection * ModelTransforms.second.front();
                m_renderPasses[RenderPassTargets.first].NewOnRendorObj(m_modelDatas.at(ModelTransforms.first).m_vertices.size(), push);
                ModelTransforms.second.pop();
            }
        }
        if(RenderPassTargets.first == 0)
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_renderPasses.at(0).m_commandBuffer);
        
        m_renderPasses[RenderPassTargets.first].NewOnRenderFinish();
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_renderPasses[0].m_commandBuffer;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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

void Renderer::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("Queening.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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
    allocInfo.commandPool = m_renderPasses[0].m_commandPool;
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

    vkFreeCommandBuffers(m_device, m_renderPasses[0].m_commandPool, 1, &CommandBuffer);
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
    m_imageVeiw = CreateImageView(m_imageDatas[0], VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView Renderer::CreateImageView(VkImage Image, VkFormat Format)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = Format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
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
        //it.second.m_vertexBufferData.clear(m_device);
        vkDestroyBuffer(m_device, it.second.m_vertexBuffer, nullptr);
        vkFreeMemory(m_device, it.second.m_vertexBufferMemory, nullptr);
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

    vkDestroySampler(m_device, m_sampler, nullptr);
    vkDestroyImageView(m_device, m_imageVeiw, nullptr);

    for (auto& it : m_renderPasses)
    {
        vkResetDescriptorPool(m_device, it.second.m_descriptorPool, {});
        vkDestroyPipeline(m_device, it.second.m_pipeline, nullptr);
        vkDestroyPipelineCache(m_device, it.second.m_pipelineCache, nullptr);
        vkDestroyPipelineLayout(m_device, it.second.m_pipelineLayout, nullptr);
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
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    vkDestroyCommandPool(m_device, m_pool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);

#if !defined( NDEBUG )
    //instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
    glfwDestroyWindow(m_window);

    glfwTerminate();

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

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
