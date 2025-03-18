#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "RenderingLayer/SwapChainManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

SwapChainManager::SwapChainManager(ResourceRegistry& rrRef,
    VkDevice d, VkPhysicalDevice pd, Utils::QueueFamilyIndices qfi,
    std::function<std::vector<VkImageView>(const std::vector<VkImage>&, VkFormat)> createSwapChainImageViewsF,
    std::function<std::pair<VkImage, VkImageView>(VkExtent2D)> createDepthAttachmentImageF)
    : createDepthAttachmentImageFunc(createDepthAttachmentImageF)
    , createSwapChainImageViewsFunc(createSwapChainImageViewsF)
    , device(d)
    , physicalDevice(pd)
    , queueFamilyIndices(qfi)
    , rrRef(rrRef)
    , swapChain(VK_NULL_HANDLE)
    , swapChainDepthImage(VK_NULL_HANDLE)
    , swapChainDepthImageView(VK_NULL_HANDLE)
    , swapChainImages({})
    , swapChainImageFormat(VK_FORMAT_UNDEFINED)
{
}

SwapChainManager::~SwapChainManager()
{
    CleanUpSwapChain();
}

void SwapChainManager::CreateSwapChain()
{
    //SwapChain生成に関係するサポート情報を取得
    auto surface = rrRef.FindResource<VkSurfaceKHR>(RegistryKeys::Surface);
    Utils::SurfaceCapabilities surfaceSupport = Utils::QuerySurfaceSupport(physicalDevice, surface);

    //生成するSwapChainの設定を選択
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(surfaceSupport.formats);
    VkPresentModeKHR presentMode     = ChooseSwapChainPresentMode(surfaceSupport.presentModes);
    VkExtent2D extent                = ChooseSwapChainExtent(surfaceSupport.capabilities);

    //用いる画像数を決定
    uint32_t imageCount = surfaceSupport.capabilities.minImageCount + 1;
    if (surfaceSupport.capabilities.maxImageCount > 0 &&
        imageCount > surfaceSupport.capabilities.maxImageCount)
    {
        imageCount = surfaceSupport.capabilities.maxImageCount;
    }

    //生成するSwapChainの設定
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1; //ステレオスコピック3D開発でもなければ1にする
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //キューファミリーの設定(所有権の扱い次第で最適化可)
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
    {
        //画像の所有権を2つのファミリーで共有
        uint32_t grapFValue = queueFamilyIndices.graphicsFamily.value();
        uint32_t presFValue = queueFamilyIndices.presentFamily.value();
        uint32_t queueFamilyIndices[] = 
        {
            grapFValue,
            presFValue
        };
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = surfaceSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    //SwapChain生成、ResourceRegistryに登録
    VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("SwapChainManager: failed to create swap chain!");
    }
    rrRef.RegisterResource(RegistryKeys::SwapChain, swapChain);

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    rrRef.RegisterResource<VkExtent2D>(RegistryKeys::SwapChainExtent, extent);

    //SwapChainに対応するImageViewと深度バッファを生成
    swapChainImageViews     = createSwapChainImageViewsFunc(swapChainImages, swapChainImageFormat);
    auto depthAttachment    = createDepthAttachmentImageFunc(extent);
    swapChainDepthImage     = depthAttachment.first;
    swapChainDepthImageView = depthAttachment.second;
}

void SwapChainManager::RecreateSwapChain()
{
    //最小化時は処理を停止する
    int width = 0, height = 0;
    auto window = rrRef.FindResource<GLFWwindow*>(RegistryKeys::Window);
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    //既存のSwapChainを破棄し、新たに生成
    CleanUpSwapChain();
    CreateSwapChain();
}

void SwapChainManager::CleanUpSwapChain()
{
    for (auto& imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
    swapChainImageViews.clear();

    if (swapChainDepthImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, swapChainDepthImage, nullptr);
        swapChainDepthImage = VK_NULL_HANDLE;
    }
    if (swapChainDepthImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, swapChainDepthImageView, nullptr);
        swapChainDepthImageView = VK_NULL_HANDLE;
    }

    if (swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

VkExtent2D SwapChainManager::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        auto window = rrRef.FindResource<GLFWwindow*>(RegistryKeys::Window);
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkSurfaceFormatKHR SwapChainManager::ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainManager::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}