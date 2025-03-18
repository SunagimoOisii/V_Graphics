#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <array>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    alignas(16) glm::vec3 normal;

    //�V�F�[�_�v���O������GPU��̒��_�f�[�^���擾
    static VkVertexInputBindingDescription GetBindingDescription()
    {
        //�擾�̌`��
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding   = 0; //�o�C���f�B���O�z��̃C���f�b�N�X
        bindingDescription.stride    = sizeof(Vertex); //1�ڂ��玟�̃G���g���܂ł̃o�C�g��
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    //���_�f�[�^���璸�_����(�ʒu��F)���擾
    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        //�擾�̕��@�ɂ��ďڍד���
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        //pos
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0; //�V�F�[�_�v���O������location�f�B���N�e�B�u�ŎQ��
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos); //���_���Ƃ̃f�[�^�̐擪����̃o�C�g��
        //color
        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);
        //texCoord
        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);
        //normal
        attributeDescriptions[3].binding  = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset   = offsetof(Vertex, normal);

        return attributeDescriptions;
    }
};