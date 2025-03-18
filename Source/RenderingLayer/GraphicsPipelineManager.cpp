#include <stdexcept>
#include <fstream>
#include <iostream>

#include "RenderingLayer/GraphicsPipelineManager.h"
#include "ResourceRegistry.h"
#include "Utilities.h"
#include "Vertex.h"

//************************************************************************************************************************
// public��
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
    //�V�F�[�_���W���[���쐬
    auto vertShaderCode = Utils::ReadFile(vertexShaderPath);
    auto fragShaderCode = Utils::ReadFile(fragmentShaderPath);
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    //�o�[�e�b�N�X�V�F�[�_�X�e�[�W
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName  = "main"; //�G���g���[�|�C���g
    //vertShaderStageInfo.pSpecializationInfo = ���ꉻ�萔

//�t���O�����g�V�F�[�_�X�e�[�W
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

    //���_�f�[�^
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //�V�F�[�_�v���O��������̃f�[�^�y�ю󂯎��̌`���擾
    auto bindingDescription    = Vertex::GetBindingDescription();
    auto attributeDescriptoins = Vertex::GetAttributeDescriptions();

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptoins.size());
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptoins.data();

    //���̓A�Z���u��
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //���_����`�悳���W�I���g���̎��
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //�r���[�|�[�g
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    //���X�^���C�U
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE; //�V���h�E�}�b�v�g�p���ɗL��
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL; //�W�I���g���̃t���O�����g�������@
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_NONE; //���ʂ��f��
    //std::cout << "GraphicsPipelineManager: ���݁A�J�����O���L���ɂȂ��Ă��܂���" << std::endl;
    //rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT; //���ʃJ�����O
    //rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE; //���ʃJ�����O
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    //�}���`�T���v�����O
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    //�[�x�e�X�g
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable  = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE; //�[�x�e�X�g��ʉ߂����t���O�����g�̐[�x��[�x�o�b�t�@�ɏ������ނ�
    depthStencil.depthCompareOp   = VK_COMPARE_OP_LESS; //�[�x��r�Ő[�x�̍���̂ǂ����D�悷�邩
    //�[�x���E�e�X�g
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f;
    depthStencil.maxDepthBounds        = 1.0f;
    //�X�e���V��
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back  = {};

    //�J���[�u�����f�B���O
        //�t���[���o�b�t�@���Ƃ̐ݒ�
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    //�J���[�u�����f�B���O�̕��@�̐ݒ�
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment; //�t���[���o�b�t�@���Ƃ̐ڑ�
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    //���I�X�e�[�g
        //���I��Ԃɂ���ΒP��R�}���h�o�b�t�@���ňقȂ�r���[�|�[�g,�V�U�[��`���w��\
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT
       ,VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates    = dynamicStates.data();

    //�p�C�v���C�����C�A�E�g
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts = { descriptorSetLayout };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount          = 1;
    pipelineLayoutInfo.pSetLayouts             = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount  = 0;
    pipelineLayoutInfo.pPushConstantRanges     = nullptr;

    //���C�A�E�g�쐬�AResourceRegistry�ɓo�^
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

    //�����O���t�B�b�N�X�p�C�v���C���̏ڍד���
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = shaderStages;
    //�Œ�@�\�X�e�[�W�̎Q��
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicStateInfo;
    //���C�A�E�g�ƃ����_�[�p�X��Renderer�N���X�̃o���h���ő��
    pipelineInfo.layout     = pipelineLayout;
    pipelineInfo.renderPass = rrRef.FindResource<VkRenderPass>(RegistryKeys::RenderPass);
    pipelineInfo.subpass    = 0; //�T�u�p�X�̃C���f�b�N�X
    //�����̃p�C�v���C������V���ȃ��m���쐬����ۂɎg�p
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    //�O���t�B�b�N�X�p�C�v���C�������AResourceRegistry�ɓo�^
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
// private��
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