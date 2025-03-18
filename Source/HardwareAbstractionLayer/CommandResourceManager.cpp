#include <stdexcept>
#include <iostream>

#include "HardwareAbstractionLayer/CommandResourceManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
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
    //コマンドバッファ解放
    if (!commandBuffers.empty())
    {
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
        std::cout << "CommandResourceManager: All command buffers have been destroyed." << std::endl;
    }

    //コマンドプール解放
    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        std::cout << "CommandResourceManager: Command pool destroyed." << std::endl;
    }
}

void CommandResourceManager::CreateCommandPool()
{
    //作成するコマンドプールの設定
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //コマンドプールは1種類のキューに使用するコマンドバッファのみを割り当て可能
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    //コマンドプール作成、ResourceRegistryに登録
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
    //コマンドバッファ数を最大フレーム数に合わせる
    commandBuffers.resize(maxFramesInFlight);

    //作成コマンドバッファの設定
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool        = commandPool; //割当先指定
    allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    //コマンドバッファ作成、ResourceRegistryに登録
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