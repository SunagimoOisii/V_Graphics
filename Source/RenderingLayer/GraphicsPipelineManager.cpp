#include <stdexcept>
#include <fstream>
#include <iostream>

#include "RenderingLayer/GraphicsPipelineManager.h"
#include "ResourceRegistry.h"
#include "Utilities.h"
#include "Vertex.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

GraphicsPipelineManager::GraphicsPipelineManager(ResourceRegistry& rrRef,
    VkDevice device, VkDescriptorSetLayout dSetLayout)
    : device(device)
    , descriptorSetLayout(dSetLayout)
    , graphicsPipeline(VK_NULL_HANDLE)
    , pipelineLayout(VK_NULL_HANDLE)
    , rrRef(rrRef)
{
}

GraphicsPipelineManager::~GraphicsPipelineManager()
{
    if (graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
    }
    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}

void GraphicsPipelineManager::CreateGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    //シェーダモジュール作成
    auto vertShaderCode = Utils::ReadFile(vertexShaderPath);
    auto fragShaderCode = Utils::ReadFile(fragmentShaderPath);
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    //バーテックスシェーダステージ
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName  = "main"; //エントリーポイント
    //vertShaderStageInfo.pSpecializationInfo = 特殊化定数

//フラグメントシェーダステージ
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vertShaderStageInfo
       ,fragShaderStageInfo
    };

    //頂点データ
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //シェーダプログラムからのデータ及び受け取りの形式取得
    auto bindingDescription    = Vertex::GetBindingDescription();
    auto attributeDescriptoins = Vertex::GetAttributeDescriptions();

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptoins.size());
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptoins.data();

    //入力アセンブリ
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //頂点から描画されるジオメトリの種類
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //ビューポート
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    //ラスタライザ
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE; //シャドウマップ使用時に有効
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL; //ジオメトリのフラグメント生成方法
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_NONE; //両面を映す
    //std::cout << "GraphicsPipelineManager: 現在、カリングが有効になっていません" << std::endl;
    //rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT; //正面カリング
    //rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE; //裏面カリング
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    //マルチサンプリング
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    //深度テスト
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable  = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE; //深度テストを通過したフラグメントの深度を深度バッファに書き込むか
    depthStencil.depthCompareOp   = VK_COMPARE_OP_LESS; //深度比較で深度の高低のどちらを優先するか
    //深度境界テスト
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f;
    depthStencil.maxDepthBounds        = 1.0f;
    //ステンシル
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back  = {};

    //カラーブレンディング
        //フレームバッファごとの設定
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    //カラーブレンディングの方法の設定
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment; //フレームバッファ側との接続
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    //動的ステート
        //動的状態にすれば単一コマンドバッファ内で異なるビューポート,シザー矩形を指定可能
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT
       ,VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates    = dynamicStates.data();

    //パイプラインレイアウト
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts = { descriptorSetLayout };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount          = 1;
    pipelineLayoutInfo.pSetLayouts             = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount  = 0;
    pipelineLayoutInfo.pPushConstantRanges     = nullptr;

    //レイアウト作成、ResourceRegistryに登録
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("GraphicsPipelineManager: failed to create PipelineLayout!");
    }
    else
    {
        std::cout << "GraphicsPipelineManager: Created PipelineLayout" << std::endl;
        rrRef.RegisterResource(RegistryKeys::GraphicsPipelineLayout, pipelineLayout);
    }

    //生成グラフィックスパイプラインの詳細入力
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = shaderStages;
    //固定機能ステージの参照
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicStateInfo;
    //レイアウトとレンダーパスはRendererクラスのバンドルで代入
    pipelineInfo.layout     = pipelineLayout;
    pipelineInfo.renderPass = rrRef.FindResource<VkRenderPass>(RegistryKeys::RenderPass);
    pipelineInfo.subpass    = 0; //サブパスのインデックス
    //既存のパイプラインから新たなモノを作成する際に使用
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    //グラフィックスパイプライン生成、ResourceRegistryに登録
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("GraphicsPipelineManager: failed to create GraphicsPipeline!");
    }
    else
    {
        std::cout << "GraphicsPipelineManager: Created GraphicsPipeline" << std::endl;
        rrRef.RegisterResource(RegistryKeys::GraphicsPipeline, graphicsPipeline);
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

VkShaderModule GraphicsPipelineManager::CreateShaderModule(const std::vector<char>& code) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}