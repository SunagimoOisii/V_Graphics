#pragma once
#include <vulkan/vulkan.h>

#include "Utilities.h"

/// <summary>
/// �R�}���h�v�[���ƃt���[�����Ƃ̃R�}���h�o�b�t�@�̃��C�t�T�C�N�����Ǘ�����N���X<para></para>
/// �ꎞ�I�ȃR�}���h����(Begin/End)�͔ėp�֐�(Utils���O���)�ŏ�������邽�߁A
/// �{�N���X�͎�Ƀv�[����o�b�t�@�̐����ƃt���[���ԊǗ���S���B
/// </summary>
class CommandResourceManager
{
public:
    CommandResourceManager(class ResourceRegistry& rrRef, int maxFramesInFlight,
        VkDevice d, VkQueue graphicsQ, Utils::QueueFamilyIndices qfi);
    ~CommandResourceManager();

//***********************************************************
// �֐���
//***********************************************************
    
    void CreateCommandPool();
    void CreateCommandBuffers();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    int maxFramesInFlight;

    VkDevice device;
    VkQueue  graphicsQueue;
    Utils::QueueFamilyIndices queueFamilyIndices;

    VkCommandPool                commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    class ResourceRegistry& rrRef;
};