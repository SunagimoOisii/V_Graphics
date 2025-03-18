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

    //シェーダプログラムがGPU上の頂点データを取得
    static VkVertexInputBindingDescription GetBindingDescription()
    {
        //取得の形式
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding   = 0; //バインディング配列のインデックス
        bindingDescription.stride    = sizeof(Vertex); //1つ目から次のエントリまでのバイト数
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    //頂点データから頂点属性(位置や色)を取得
    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        //取得の方法について詳細入力
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        //pos
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0; //シェーダプログラムのlocationディレクティブで参照
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos); //頂点ごとのデータの先頭からのバイト数
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