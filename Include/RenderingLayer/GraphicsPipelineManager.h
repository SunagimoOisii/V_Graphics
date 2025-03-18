#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// �O���t�B�b�N�X�p�C�v���C���̐����ƊǗ����s���N���X
/// </summary>
class GraphicsPipelineManager
{
public:
    GraphicsPipelineManager(class ResourceRegistry& rrRef,
        VkDevice device, VkDescriptorSetLayout dSetLayout);
    ~GraphicsPipelineManager();

//***********************************************************
// �֐���
//***********************************************************

    void CreateGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkDevice device;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipeline       graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    std::function<VkDescriptorSetLayout()> getDescriptorSetLayoutFunc;

    class ResourceRegistry& rrRef;

//***********************************************************
// �֐���
//***********************************************************

    VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
};