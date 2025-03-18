#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

#include "Utilities.h"

/// <summary>
/// 物理・論地デバイス、キューの管理を行う<para></para>
/// なおデバイスでサポートされているメモリタイプやフォーマットの検索、
/// サーフェスの機能サポートクエリは責任外
/// </summary>
class DeviceManager
{
public:
    DeviceManager(class ResourceRegistry& rrRef,
        VkInstance ins, VkSurfaceKHR sur);
    ~DeviceManager();

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;

    Utils::QueueFamilyIndices indices;

    const std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
    };

    class ResourceRegistry& rrRef;

//***********************************************************
// 関数↓
//***********************************************************

    bool IsDeviceSuitable(VkPhysicalDevice pDev);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice pDev);

    /// <summary>
    /// 引数デバイスのキューファミリーを検索する
    /// </summary>
    /// <returns>見つかったキューファミリーのインデックス</returns>
    Utils::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pDev) const;

    void PickPhysicalDevice();

    void CreateLogicalDevice();
};