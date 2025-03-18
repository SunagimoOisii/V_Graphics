#pragma once
#include <memory>
#include <vulkan/vulkan.h>

#include "VulkanInitializeManager.h"
#include "WindowManager.h"

/// <summary>
/// プラットフォーム依存部分を統合し、必要なリソースを提供するレイヤーラップクラス<para></para>
/// ウィンドウ管理やVulkanインスタンスの生成を担当し、<para></para>
/// HardwareAbstractionLayerに必要なVkInstanceやVkSurfaceKHRを提供する
/// </summary>
class SystemIntegrationLayer
{
public:
    SystemIntegrationLayer(class ResourceRegistry& rrRef,
        bool enableValidationLayers,
        uint32_t width, uint32_t height, const char* windowTitle);
    ~SystemIntegrationLayer();

private:
//***********************************************************
// 変数↓
//***********************************************************

    std::unique_ptr<VulkanInitializeManager> vulkanInitializeManager;
    std::unique_ptr<WindowManager>           windowManager;
};