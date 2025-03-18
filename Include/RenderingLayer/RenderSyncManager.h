#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>

/// <summary>
/// �����_�����O�v���Z�X�ɂ�����Z�}�t�H�ƃt�F���X���Ǘ�����N���X<para></para>
/// �X���b�v�`�F�C���̃t���[�����Ƃɓ����I�u�W�F�N�g�𐶐��E�Ǘ����A
/// �����_�����O�̏����Ɗ����𐧌䂷��
/// </summary>
class RenderSyncManager
{
public:
    RenderSyncManager(VkDevice d, int maxFramesInFlight);
    ~RenderSyncManager();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    VkSemaphore GetImageAvailableSemaphore(int currentFrame) const
    {
        return imageAvailableSemaphores[currentFrame];
    }
    VkSemaphore GetRenderFinishedSemaphore(int currentFrame) const
    {
        return renderFinishedSemaphores[currentFrame];
    }
    VkFence     GetInFlightFence(int currentFrame) const
    {
        return inFlightFences[currentFrame];
    }

//***********************************************************
// �֐���
//***********************************************************

    void CreateSyncObjects();

    void WaitForInFlightFence(int currentFrame);
    void ResetInFlightFence(int currentFrame);
    void SetMaxFramesInFlight(int newMaxFrames);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    int maxFramesInFlight;
    
    VkDevice device;

    //�X���b�v�`�F�C������摜�擾�������_�����O����������m�点��
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //�����_�����O���I�����v���[���e�[�V�����\��Ԃ�m�点��
    std::vector<VkSemaphore> renderFinishedSemaphores;

    //��x��1�t���[���̂݃����_�����O������悤�ɂ���
    std::vector<VkFence> inFlightFences;
};