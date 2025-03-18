#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

#include "SwapChainManager.h"

class RenderPassManager
{
public:
    RenderPassManager(class ResourceRegistry& rrRef, VkDevice d, VkPhysicalDevice pd,
        std::function<VkFormat()> getSwapChainImageFormatF);
    ~RenderPassManager();

//***********************************************************
// �֐���
//***********************************************************

    void CreateRenderPass();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkRenderPass renderPass;

    std::function<VkFormat()> getSwapChainImageFormatFunc;

    class ResourceRegistry& rrRef;
};