#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>

const std::vector<std::string> textureNames = { "Test" };

//ResourceRegistryクラスで各種リソースに対応したキーを定義している
namespace RegistryKeys
{
//***********************************************************
// 描画設定系
//***********************************************************

    constexpr const char* MaxFramesInFlight = "MaxFramesInFlight";

//***********************************************************
// SystemIntegrationLayer系
//***********************************************************

    constexpr const char* Window     = "Window";
    constexpr const char* Surface    = "Surface";
    constexpr const char* VkInstance = "VkInstance";

//***********************************************************
// HardwareAbstractionLayer系
//***********************************************************

    constexpr const char* Device             = "Device";
    constexpr const char* PhysicalDevice     = "PhysicalDevice";
    constexpr const char* GraphicsQueue      = "GraphicsQueue";
    constexpr const char* PresentQueue       = "PresentQueue";
    constexpr const char* QueueFamilyIndices = "QueueFamilyIndices";

    constexpr const char* CommandPool    = "CommandPool";
    constexpr const char* CommandBuffers = "CommandBuffers";

//***********************************************************
// DrawResourceLayer系
//***********************************************************

    constexpr const char* MVPUBs = "MVPUniformBuffers";

//***********************************************************
// RenderingLayer系
//***********************************************************

    constexpr const char* SwapChain       = "SwapChain";
    constexpr const char* SwapChainExtent = "SwapChainExtent";

    constexpr const char* RenderPass = "RenderPass";

    constexpr const char* GraphicsPipeline       = "GraphicsPipeline";
    constexpr const char* GraphicsPipelineLayout = "GraphicsPipelineLayout";

    constexpr const char* DescriptorSetLayout = "DescriptorSetLayout";
    constexpr const char* DescriptorSets      = "DescriptorSets";
}

namespace Utils
{
//***********************************************************
// 構造体↓
//***********************************************************

    /// <summary>
    /// サーフェス詳細情報を保持する構造体
    /// </summary>
    struct SurfaceCapabilities
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /// <summary>
    /// キューファミリーのインデックスを保持する構造体
    /// </summary>
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

//****************************
// UniformBufferObject↓
//****************************
    //各シェーダステージがディスクリプタ越しに参照する

    struct MVPUBO
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;

        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
        float intensity;
    };

    struct GlobalLightUBO
    {
        alignas(16) glm::vec3 color;
        alignas(4)  float intensity;
    };

    struct DirectionalLightUBO
    {
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
        float intensity;
    };

//***********************************************************
// 関数↓
//***********************************************************

    /// <summary>
    /// 条件に合致するメモリタイプのインデックスを取得する
    /// </summary>
    /// <param name="typeFilter">メモリタイプのビットマスク</param>
    /// <param name="properties">要求するメモリプロパティ</param>
    /// <returns>見つかったメモリタイプのインデックス</returns>
    uint32_t FindMemoryType(VkPhysicalDevice pd, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /// <summary>
    /// 物理デバイスでサポートされているフォーマットを検索する
    /// </summary>
    /// <param name="candidates">サポートしているフォーマットのリスト</param>
    /// <param name="tiling">画像のタイリング方法</param>
    /// <param name="features">必要なフォーマット機能フラグ</param>
    /// <returns>サポートされているフォーマット</returns>
    VkFormat FindSupportedFormat(VkPhysicalDevice pd, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat FindDepthFormat(VkPhysicalDevice pd);

    Utils::SurfaceCapabilities QuerySurfaceSupport(VkPhysicalDevice pd, VkSurfaceKHR sur);

    /// <summary>
    /// 一時的なコマンドバッファを開始
    /// </summary>
    /// <returns>記録可能な状態のコマンドバッファ</returns>
    VkCommandBuffer BeginTransientCommands(VkDevice d, VkCommandPool cp);

    /// <summary>
    /// 一時的なコマンドバッファの記録を終了し、キューに提出した後メモリ開放
    /// </summary>
    void EndTransientCommands(VkDevice d, VkQueue graphicsQ, VkCommandPool cp, VkCommandBuffer cb);

    std::vector<char> ReadFile(const std::string& fileName);
}