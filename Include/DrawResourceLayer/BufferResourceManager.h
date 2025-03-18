#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <functional>

#include "Vertex.h"
#include "Utilities.h"

/// <summary>
/// バッファリソースを生成・管理するクラス<para></para>
/// 頂点,インデックスバッファの生成を行い、それらの所有は呼び出し元に委ねる<para></para>
/// また、ユニフォームバッファに関しては生成・所有・更新を一括管理し、
/// フレームごとのデータ転送をサポートする
/// </summary>
class BufferResourceManager
{
public:
    BufferResourceManager(VkDevice d, VkPhysicalDevice pd, VkQueue graphicsQ,
        VkCommandPool cp, class MemoryPool& mpRef, class ResourceRegistry& rrRef);
    ~BufferResourceManager();

//***********************************************************
// 関数↓
//***********************************************************

    VkBuffer CreateVertexBuffer(const std::vector<Vertex>& vertices);
    VkBuffer CreateIndexBuffer(const std::vector<uint16_t>& indices);
    void CreateStagingBuffer(VkDeviceSize size, VkBuffer& stagingBuf, VkDeviceMemory& stagingMem, VkDeviceSize& stagingOffset);

    void CopyBufferToImage(VkBuffer buf, VkImage img, uint32_t width, uint32_t height) const;

    void CreateMVPUniformBuffers(size_t maxFramesInFlight);
    void UploadMVPUniformBuffer(uint32_t currentImg, const Utils::MVPUBO& ubo);

private:
//***********************************************************
// 変数↓
//***********************************************************

    VkDevice         device;
    VkPhysicalDevice physicalDevice;
    VkQueue          graphicsQueue;

    VkCommandPool commandPool;

    class MemoryPool&       mpRef;
    class ResourceRegistry& rrRef;

//****************************
// UniformBuffer(UB)変数↓
//   比較的データが小さく,毎フレーム更新されるのでステージングバッファを作成しない
//****************************

    std::vector<VkBuffer>       MVPUBs; 
    std::vector<VkDeviceMemory> MVPUBsMemory;

//***********************************************************
// 関数↓
//***********************************************************

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buf, VkDeviceMemory& bufMem, VkDeviceSize& offset) const;

    /// <summary>
    /// シェーダのバッファに最適なメモリにはVK_MEMORY_PROPERTY_DEVICE_LOCAL_BITがあり,これはCPUからではアクセスできない可能性がある
    /// さらにvkMapMemoryは, GPUメモリ領域をCPU側で用いるためにアクセス可能にするが, その逆はできないため,
    /// この関数でステージングバッファから各シェーダのバッファ(GPU側)へのコピーを行いGPU側でシェーダデータを扱えるようにする
    /// </summary>
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
};