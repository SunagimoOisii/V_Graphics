#pragma once
#include <memory>

#include "FrameBufferManager.h"
#include "RenderPassManager.h"
#include "GraphicsPipelineManager.h"
#include "SwapChainManager.h"
#include "RenderSyncManager.h"
#include "DescriptorManager.h"
#include "Renderer.h"

/// <summary>
/// �����_�����O�\���𓝊��Ǘ����郌�C���[���b�v�N���X<para></para>
/// �X���b�v�`�F�C���A�����_�[�p�X�A�t���[���o�b�t�@�A�O���t�B�b�N�X�p�C�v���C���A
/// �`�擯�����\�[�X�A�����_���[�̐����ƊǗ���S��
/// </summary>
class RenderingLayer
{
public:
    RenderingLayer(class ResourceRegistry& rrRef, class Camera& cam,
        VkBuffer vertexB, VkBuffer indexB, const std::vector<uint16_t> indices,
        std::function<VkImageView(std::string)> getTextureImageViewF,
        std::function<VkSampler(std::string)> getTextureSamplerF,
        std::function<std::vector<VkImageView>(const std::vector<VkImage>&, VkFormat)> createSwapChainImageViewsF,
        std::function<std::pair<VkImage, VkImageView>(VkExtent2D)> createDepthAttachmentImageF,
        std::function<void(uint32_t, const Utils::MVPUBO&)> uploadMVPUniformBuffersF);
    ~RenderingLayer();

//***********************************************************
// �֐���
//***********************************************************

    void Render(bool& loopFlag, float deltaTime);

    void CreateRenderingFlow(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    void RecreateRenderingFlow();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    std::unique_ptr<SwapChainManager>        swapChainManager;
    std::unique_ptr<RenderPassManager>       renderPassManager;
    std::unique_ptr<FrameBufferManager>      frameBufferManager;
    std::unique_ptr<GraphicsPipelineManager> graphicsPipelineManager;

    std::unique_ptr<RenderSyncManager> renderSyncManager;

    std::unique_ptr<DescriptorManager> descriptorManager;

    std::unique_ptr<class Renderer> renderer;
};