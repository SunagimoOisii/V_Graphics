#include <stdexcept>
#include <iostream>

#include "HardwareAbstractionLayer/CommandResourceManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public��
//************************************************************************************************************************

CommandResourceManager::CommandResourceManager(ResourceRegistry& rrRef, int maxFramesInFlight,
    VkDevice d, VkQueue graphicsQ, Utils::QueueFamilyIndices qfi)
    : device(d)
    , commandPool(VK_NULL_HANDLE)
    , maxFramesInFlight(maxFramesInFlight)
    , graphicsQueue(graphicsQ)
    , queueFamilyIndices(qfi)
    , rrRef(rrRef)
{
}

CommandResourceManager::~CommandResourceManager()
{
    //�R�}���h�o�b�t�@���
    if (!commandBuffers.empty())
    {
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
        std::cout << "CommandResourceManager: All command buffers have been destroyed." << std::endl;
    }

    //�R�}���h�v�[�����
    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        std::cout << "CommandResourceManager: Command pool destroyed." << std::endl;
    }
}

void CommandResourceManager::CreateCommandPool()
{
    //�쐬����R�}���h�v�[���̐ݒ�
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //�R�}���h�v�[����1��ނ̃L���[�Ɏg�p����R�}���h�o�b�t�@�݂̂����蓖�ĉ\
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    //�R�}���h�v�[���쐬�AResourceRegistry�ɓo�^
    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("CommandResourceManager: failed to create CommandPool!");
    }
    else
    {
        std::cout << "CommandResourceManager: Created CommandPool" << std::endl;
        rrRef.RegisterResource(RegistryKeys::CommandPool, commandPool);
    }
}

void CommandResourceManager::CreateCommandBuffers()
{
    //�R�}���h�o�b�t�@�����ő�t���[�����ɍ��킹��
    commandBuffers.resize(maxFramesInFlight);

    //�쐬�R�}���h�o�b�t�@�̐ݒ�
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool        = commandPool; //������w��
    allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    //�R�}���h�o�b�t�@�쐬�AResourceRegistry�ɓo�^
    VkResult result = vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("CommandResourceManager: Failed to allocate CommandBuffers!");
    }
    else
    {
        std::cout << "CommandResourceManager: Allocated CommandBuffers" << std::endl;
        rrRef.RegisterResource(RegistryKeys::CommandBuffers, commandBuffers);
    }
}