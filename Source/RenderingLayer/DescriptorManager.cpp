#include <array>

#include "RenderingLayer/DescriptorManager.h"
#include "Utilities.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public��
//************************************************************************************************************************

DescriptorManager::DescriptorManager(ResourceRegistry& rrRef,
    VkDevice d, int maxFramesInFlight,
    std::function<VkImageView(std::string)> getTextureImageViewFunc,
    std::function<VkSampler(std::string)> getTextureSamplerFunc)
    : device(d)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)    
    , getTextureImageViewFunc(getTextureImageViewFunc)
    , getTextureSamplerFunc(getTextureSamplerFunc)
    , maxFramesInFlight(maxFramesInFlight)
    , rrRef(rrRef)
{
}

DescriptorManager::~DescriptorManager()
{
    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    if (descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
}

void DescriptorManager::CreateDescriptorSetLayout()
{
    //����DescriptorSetLayout�̐ݒ�
    //MVPUniformBuffer�p�o�C���f�B���O�ݒ�
    VkDescriptorSetLayoutBinding mvpUBLB{};
    mvpUBLB.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mvpUBLB.descriptorCount    = 1;
    mvpUBLB.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    mvpUBLB.binding            = 0; //�e�V�F�[�_�X�e�[�W���̃o�C���f�B���O�ԍ��̃��\�[�X�ɑΉ�
    mvpUBLB.pImmutableSamplers = nullptr;
    //�e�N�X�`���T���v�����O�p�o�C���f�B���O�ݒ�
    VkDescriptorSetLayoutBinding samplerLB{};
    samplerLB.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLB.descriptorCount    = 1;
    samplerLB.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLB.binding            = 1;
    samplerLB.pImmutableSamplers = nullptr;

    //����DescriptorSetLayout���̂̐ݒ����
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { mvpUBLB, samplerLB };
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings    = bindings.data();

    //DescriptorSetLayout�����AResourceRegistry�ɓo�^
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("DescriptorManager: failed to Create DescriptorSetLayout!");
    }
    rrRef.RegisterResource(RegistryKeys::DescriptorSetLayout, descriptorSetLayout);
}

void DescriptorManager::CreateDescriptorPool()
{
    //����DescriptorPool�̐ݒ�
    //�w��^�C�v�ƌ��̃f�B�X�N���v�^���v�[�����Ɋ��蓖�Ă�悤�ɐݒ肷��
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    //MVPUniformBuffer�p�f�B�X�N���v�^�̐ݒ�
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
    //�e�N�X�`���T���v���[�p�f�B�X�N���v�^�̐ݒ�
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);

    //����DescriptorPool���̂̐ݒ�
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.maxSets       = static_cast<uint32_t>(maxFramesInFlight);
    poolInfo.pPoolSizes    = poolSizes.data();

    //DescriptorPool����
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("DescriptorManager: failed to create DescriptorPool!");
    }
}

void DescriptorManager::CreateDescriptorSets()
{
    //����DescriptorSets�̏��W��
    std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool     = descriptorPool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight);
    allocateInfo.pSetLayouts        = layouts.data();

    //DescriptorSets�����AResourceRegistry�ɓo�^
    descriptorSets.resize(maxFramesInFlight);
    VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("DescriptorManager: failed to allocate DescriptorSets!");
    }
    rrRef.RegisterResource(RegistryKeys::DescriptorSets, descriptorSets);

    //DescriptorSets���̊e�f�B�X�N���v�^�̃f�[�^����
    auto uniformBuffers = rrRef.FindResource<std::vector<VkBuffer>>(RegistryKeys::MVPUBs);
    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(Utils::MVPUBO);

        //�Z�b�g���̃f�B�X�N���v�^�Ƀo�C���h������C���[�W�ƃT���v���[�̃��\�[�X�ɂ���
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = getTextureImageViewFunc(textureNames[0]);
        imageInfo.sampler     = getTextureSamplerFunc(textureNames[0]);

        std::array<VkWriteDescriptorSet, 2> wdSets{};
        //MVPUniformBuffer
        wdSets[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wdSets[0].dstSet          = descriptorSets[i];
        wdSets[0].dstBinding      = 0;
        wdSets[0].dstArrayElement = 0;
        wdSets[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        wdSets[0].descriptorCount = 1;
        wdSets[0].pBufferInfo     = &bufferInfo;
        //�C���[�W,�T���v���[
        wdSets[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wdSets[1].dstSet          = descriptorSets[i];
        wdSets[1].dstBinding      = 1;
        wdSets[1].dstArrayElement = 0;
        wdSets[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        wdSets[1].descriptorCount = 1;
        wdSets[1].pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(wdSets.size()), wdSets.data(), 0, nullptr);
    }
}