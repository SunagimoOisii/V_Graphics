#pragma once
#include <vulkan/vulkan.h>

#include "Utilities.h"

/// <summary>
/// コマンドプールとフレームごとのコマンドバッファのライフサイクルを管理するクラス<para></para>
/// 一時的なコマンド操作(Begin/End)は汎用関数(Utils名前空間)で処理されるため、
/// 本クラスは主にプールやバッファの生成とフレーム間管理を担当。
/// </summary>
class CommandResourceManager
{
public:
    CommandResourceManager(class ResourceRegistry& rrRef, int maxFramesInFlight,
        VkDevice d, VkQueue graphicsQ, Utils::QueueFamilyIndices qfi);
    ~CommandResourceManager();

//***********************************************************
// 関数↓
//***********************************************************
    
    void CreateCommandPool();
    void CreateCommandBuffers();

private:
//***********************************************************
// 変数↓
//***********************************************************

    int maxFramesInFlight;

    VkDevice device;
    VkQueue  graphicsQueue;
    Utils::QueueFamilyIndices queueFamilyIndices;

    VkCommandPool                commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    class ResourceRegistry& rrRef;
};