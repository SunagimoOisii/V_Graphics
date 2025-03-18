#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

#include "SwapChainManager.h"

class FrameBufferManager
{
public:
    FrameBufferManager(class ResourceRegistry& rrRef, VkDevice d,
        std::function<const std::vector<VkImageView> ()> getSwapChainImageViewsF,
        std::function<VkImageView()> getDepthImageViewF);
    ~FrameBufferManager();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    const std::vector<VkFramebuffer>& GetFrameBuffers() const { return frameBuffers; }

//***********************************************************
// 関数↓
//***********************************************************

    void CreateFrameBuffers();

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkDevice device;

    std::vector<VkFramebuffer> frameBuffers;

    std::function<const std::vector<VkImageView> ()> getSwapChainImageViewsFunc;
    std::function<VkImageView()> getDepthImageViewFunc;

    class ResourceRegistry& rrRef;

//***********************************************************
// 関数↓
//***********************************************************

    void CleanUpFrameBuffers();
};