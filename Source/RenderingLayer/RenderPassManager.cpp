#include <stdexcept>
#include <iostream>
#include <array>

#include "RenderingLayer/RenderPassManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public��
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
    //�A�^�b�`�����g(�T�u�p�X�Ԃł���肳���f�[�^)�̐ݒ�
    //�J���[�A�^�b�`�����g�̐ݒ�
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = getSwapChainImageFormatFunc();
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; //�����̃A�^�b�`�����g������
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //�T�u�p�X����̎Q�ƂɎg�p
    //�����_�[�p�X�ւ̐ڑ��͍ė��p������̂��߃T�u�p�X��ʂ��čs��
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment  = 0;
    colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //�J���[�o�b�t�@�Ƃ��Ďg�p

    //�[�x�A�^�b�`�����g�̐ݒ�
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

    //�T�u�p�X�̐ݒ�(�A�^�b�`�����g����p���ă����_�[�p�X���ŏ������s��)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef; //�����̃A�^�b�`�����g���󂯕t���\
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    //�T�u�p�X�̈ˑ��֌W�̐ݒ�
    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0; //�ˑ��T�u�p�X�̃C���f�b�N�X�w��
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    //���삪��������X�e�[�W�w��
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    //�����_�[�p�X���̂̐ݒ�
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

    //RenderPass�𐶐����AResourceRegistry�ɓo�^
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