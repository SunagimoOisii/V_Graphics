#include <stdexcept>
#include <iostream>
#include <array>

#include "RenderingLayer/RenderPassManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

RenderPassManager::RenderPassManager(ResourceRegistry& rrRef, VkDevice d, VkPhysicalDevice pd,
    std::function<VkFormat()> getSwapChainImageFormatF)
    : device(d)
    , getSwapChainImageFormatFunc(getSwapChainImageFormatF)
    , physicalDevice(pd)
    , renderPass(VK_NULL_HANDLE)
    , rrRef(rrRef)
{
}

RenderPassManager::~RenderPassManager()
{
    if (renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }
}

void RenderPassManager::CreateRenderPass()
{
    //アタッチメント(サブパス間でやり取りされるデータ)の設定
    //カラーアタッチメントの設定
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = getSwapChainImageFormatFunc();
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; //複数のアタッチメントを持つ
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //サブパスからの参照に使用
    //レンダーパスへの接続は再利用性向上のためサブパスを通して行う
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment  = 0;
    colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //カラーバッファとして使用

    //深度アタッチメントの設定
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = Utils::FindDepthFormat(physicalDevice);
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //サブパスの設定(アタッチメント情報を用いてレンダーパス内で処理を行う)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef; //複数のアタッチメントを受け付け可能
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    //サブパスの依存関係の設定
    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0; //依存サブパスのインデックス指定
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    //操作が発生するステージ指定
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    //レンダーパス自体の設定
    std::array<VkAttachmentDescription, 2> attachments = 
    {
        colorAttachment,
        depthAttachment 
    };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    //RenderPassを生成し、ResourceRegistryに登録
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("RenderPassManager: failed to create render pass!");
    }
    else
    {
        std::cout << "RenderPassManager: Created RenderPass" << std::endl;
        rrRef.RegisterResource<VkRenderPass>(RegistryKeys::RenderPass, renderPass);
    }
}