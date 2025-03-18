#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <functional>

/// <summary>
/// �f�B�X�N���v�^�Z�b�g�̃��C�A�E�g�A�v�[���A�Z�b�g�̐����ƊǗ����s���N���X
/// </summary>
class DescriptorManager
{
public:
    DescriptorManager(class ResourceRegistry& rrRef,
        VkDevice d, int maxFramesInFlight,
        std::function<VkImageView(std::string)> getTextureImageViewFunc,
        std::function<VkSampler(std::string)> getTextureSamplerFunc);
    ~DescriptorManager();

//***********************************************************
// �֐���
//***********************************************************

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    int maxFramesInFlight;

    VkDevice device;

    VkDescriptorPool             descriptorPool;
    VkDescriptorSetLayout        descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::function<VkImageView(std::string)> getTextureImageViewFunc;
    std::function<VkSampler(std::string)>   getTextureSamplerFunc;

    class ResourceRegistry& rrRef;
};