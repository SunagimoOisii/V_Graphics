#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <functional>

/// <summary>
/// ディスクリプタセットのレイアウト、プール、セットの生成と管理を行うクラス
/// </summary>
class DescriptorManager
{
public:
    DescriptorManager(class ResourceRegistry& rrRef,
        VkDevice d, int maxFramesInFlight,
        std::function<VkImageView(std::string)> getTextureImageViewFunc,
        std::function<VkSampler(std::string)> getTextureSamplerFunc);
    ~DescriptorManager();

//***********************************************************
// 関数↓
//***********************************************************

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

private:
//***********************************************************
// 変数↓
//***********************************************************

    int maxFramesInFlight;

    VkDevice device;

    VkDescriptorPool             descriptorPool;
    VkDescriptorSetLayout        descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::function<VkImageView(std::string)> getTextureImageViewFunc;
    std::function<VkSampler(std::string)>   getTextureSamplerFunc;

    class ResourceRegistry& rrRef;
};