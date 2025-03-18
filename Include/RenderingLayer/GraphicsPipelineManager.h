#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// グラフィックスパイプラインの生成と管理を行うクラス
/// </summary>
class GraphicsPipelineManager
{
public:
    GraphicsPipelineManager(class ResourceRegistry& rrRef,
        VkDevice device, VkDescriptorSetLayout dSetLayout);
    ~GraphicsPipelineManager();

//***********************************************************
// 関数↓
//***********************************************************

    void CreateGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkDevice device;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipeline       graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    std::function<VkDescriptorSetLayout()> getDescriptorSetLayoutFunc;

    class ResourceRegistry& rrRef;

//***********************************************************
// 関数↓
//***********************************************************

    VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
};