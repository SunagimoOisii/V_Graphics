#include <stdexcept>
#include <iostream>
#include <array>

#include "RenderingLayer/FrameBufferManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

FrameBufferManager::FrameBufferManager(ResourceRegistry& rrRef, VkDevice d,
    std::function<const std::vector<VkImageView>()> getSwapChainImageViewsF,
    std::function<VkImageView()> getDepthImageViewF)
    : device(d)
    , getDepthImageViewFunc(getDepthImageViewF)
    , getSwapChainImageViewsFunc(getSwapChainImageViewsF)
    , rrRef(rrRef)
{
}

FrameBufferManager::~FrameBufferManager()
{
    CleanUpFrameBuffers();
}

void FrameBufferManager::CreateFrameBuffers()
{
    //既にFrameBufferが生成されていれば破棄
    CleanUpFrameBuffers();

    auto& swapChainImageViews  = getSwapChainImageViewsFunc();
    auto swapChainExtent       = rrRef.FindResource<VkExtent2D>(RegistryKeys::SwapChainExtent);
    VkImageView depthImageView = getDepthImageViewFunc();

    //スワップチェーンの全画像に対してフレームバッファを作成
    frameBuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageView };

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        //互換性を持ちたいレンダーパスを指定
        frameBufferInfo.renderPass      = rrRef.FindResource<VkRenderPass>(RegistryKeys::RenderPass);
        frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        frameBufferInfo.pAttachments    = attachments.data();
        frameBufferInfo.width           = swapChainExtent.width;
        frameBufferInfo.height          = swapChainExtent.height;
        frameBufferInfo.layers          = 1;

        VkResult result = vkCreateFramebuffer(device, &frameBufferInfo, nullptr, &frameBuffers[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("FrameBufferManager: failed to create framebuffer!");
        }
        else
        {
            std::cout << "FrameBufferManager: Created FrameBuffer" << std::endl;
        }
    }
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void FrameBufferManager::CleanUpFrameBuffers()
{
    for (auto framebuffer : frameBuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    frameBuffers.clear();
}