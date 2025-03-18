#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

/// <summary>
/// Vulkanインスタンスとデバッグメッセンジャーを管理する
/// </summary>
class VulkanInitializeManager
{
public:
    VulkanInitializeManager(bool enableValidationLayers, class ResourceRegistry& rrRef);
    ~VulkanInitializeManager();

//***********************************************************
// 関数↓
//***********************************************************

    void Initialize();

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableValidationLayers;
    const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    class ResourceRegistry& rrRef;

//***********************************************************
// 関数↓
//***********************************************************

    void CreateInstance();

    bool CheckValidationLayerSupport();

    /// <summary>
    /// 作成するデバッグメッセンジャーの設定を行う
    /// </summary>
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void SetupDebugMessenger();

    std::vector<const char*> FetchRequiredExtensions() const;
};