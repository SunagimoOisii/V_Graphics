#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

#include "Utilities.h"

class SwapChainManager
{
public:
    SwapChainManager(class ResourceRegistry& rrRef,
        VkDevice d, VkPhysicalDevice pd, Utils::QueueFamilyIndices qfi,
        std::function<std::vector<VkImageView> (const std::vector<VkImage>&, VkFormat)> createSwapChainImageViewsF,
        std::function<std::pair<VkImage, VkImageView>(VkExtent2D)> createDepthAttachmentImageF);
    ~SwapChainManager();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    VkFormat GetSwapChainImageFormat() const { return swapChainImageFormat; }
    const std::vector<VkImageView>& GetSwapChainImageViews() const { return swapChainImageViews; }
    const VkImageView GetSwapChainDepthImageView() const { return swapChainDepthImageView; }

//***********************************************************
// 関数↓
//***********************************************************

    void CreateSwapChain();
    void RecreateSwapChain();
    void CleanUpSwapChain();

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkDevice         device;
    VkPhysicalDevice physicalDevice;

    Utils::QueueFamilyIndices queueFamilyIndices;

    VkSwapchainKHR           swapChain;
    std::vector<VkImage>     swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkImage                  swapChainDepthImage;
    VkImageView              swapChainDepthImageView;
    VkFormat                 swapChainImageFormat;

    class ResourceRegistry& rrRef;

    std::function<std::vector<VkImageView>(const std::vector<VkImage>&, VkFormat)> createSwapChainImageViewsFunc;
    std::function<std::pair<VkImage, VkImageView>(VkExtent2D)> createDepthAttachmentImageFunc;

//***********************************************************
// 関数↓
//***********************************************************

    VkExtent2D         ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};