#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "DeviceManager.h"
#include "CommandResourceManager.h"

/// <summary>
/// ハードウェアリソースを統括管理するレイヤーラップクラス<para></para>
/// Vulkanデバイス、メモリ、コマンドリソースを初期化および提供する役割を担う<para></para>
/// SystemIntegrationLayerで初期化されたVulkanインスタンスやサーフェスをもとに、<para></para>
/// デバイス関連リソースを抽象化する
/// </summary>
class HardwareAbstractionLayer
{
public:
    HardwareAbstractionLayer(class ResourceRegistry& rrRef);
    ~HardwareAbstractionLayer();

private:
//***********************************************************
// 変数↓
//***********************************************************

    std::unique_ptr<DeviceManager>           deviceManager;
    std::unique_ptr<CommandResourceManager>  commandResourceManager;
};