#include "RenderingLayer/RenderSyncManager.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

RenderSyncManager::RenderSyncManager(VkDevice d, int maxFramesInFlight)
    : device(d)
    , maxFramesInFlight(maxFramesInFlight)
{
}

RenderSyncManager::~RenderSyncManager()
{
    for (int i = 0; i < maxFramesInFlight; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
}

void RenderSyncManager::CreateSyncObjects()
{
    imageAvailableSemaphores.resize(maxFramesInFlight);
    renderFinishedSemaphores.resize(maxFramesInFlight);
    inFlightFences.resize(maxFramesInFlight);

    //セマフォ準備
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //フェンス準備
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //セマフォとフェンスの生成
    for (int i = 0; i < maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("RenderSyncManager: Failed to create synchronization objects for a frame!");
        }
        else
        {
            std::cout << "RenderSyncManager: Created SyncObjects(Semaphore,fence)" << std::endl;
        }
    }
}

void RenderSyncManager::WaitForInFlightFence(int currentFrame)
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

void RenderSyncManager::ResetInFlightFence(int currentFrame)
{
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
}

void RenderSyncManager::SetMaxFramesInFlight(int newMaxFrames)
{
    //現在の同期オブジェクトを破棄してから
    //新しいフレーム数に合わせて再生成
    for (int i = 0; i < maxFramesInFlight; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    maxFramesInFlight = newMaxFrames;
    CreateSyncObjects();
}