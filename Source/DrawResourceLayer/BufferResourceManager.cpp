#include <chrono>

#include "DrawResourceLayer/BufferResourceManager.h"
#include "DrawResourceLayer/MemoryPool.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

BufferResourceManager::BufferResourceManager(VkDevice d, VkPhysicalDevice pd, VkQueue graphicsQ,
    VkCommandPool cp, MemoryPool& mpRef, ResourceRegistry& rrRef)
    : commandPool(cp)
    , device(d)
    , mpRef(mpRef)
    , physicalDevice(pd)
    , rrRef(rrRef)
    , graphicsQueue(graphicsQ)
{
}

BufferResourceManager::~BufferResourceManager()
{
    //ユニフォームバッファの削除
    for (size_t i = 0; i < MVPUBs.size(); i++)
    {
        vkDestroyBuffer(device, MVPUBs[i], nullptr);
        mpRef.Free(MVPUBsMemory[i], 0, 0);
    }
}

VkBuffer BufferResourceManager::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
    //ステージングバッファ作成(CPUとGPUの中継用, ホスト可視メモリ)
    VkDeviceSize   bufSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer       stagingBuf;
    VkDeviceMemory stagingMem;
    VkDeviceSize   stagingOffset = 0;
    CreateBuffer(bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuf, stagingMem, stagingOffset);

    //頂点データをステージングバッファにコピー
    void* data = mpRef.MapMemory(stagingMem, stagingOffset, bufSize);
    memcpy(data, vertices.data(), (size_t)bufSize);
    mpRef.UnmapMemory(stagingMem, stagingOffset, bufSize);

    //メモリ転送操作で目的地にできるバッファを作成 (他のバッファ作成関数はbuffersizeとここのusageが異なる)
    VkBuffer       vertexBuf;
    VkDeviceMemory vertexMem;
    VkDeviceSize   vertexOffset = 0;
    CreateBuffer(bufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuf, vertexMem, vertexOffset);

    CopyBuffer(stagingBuf, vertexBuf, bufSize);

    vkDestroyBuffer(device, stagingBuf, nullptr);
    mpRef.Free(stagingMem, stagingOffset, bufSize);

    return vertexBuf;
}

VkBuffer BufferResourceManager::CreateIndexBuffer(const std::vector<uint16_t>& indices)
{
    //ステージングバッファ作成 (CPUとGPUの中継用, ホスト可視メモリ)
    VkDeviceSize   bufSize = sizeof(indices[0]) * indices.size();
    VkBuffer       stagingBuf;
    VkDeviceMemory stagingMem;
    VkDeviceSize   stagingOffset = 0;
    CreateBuffer(bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuf, stagingMem, stagingOffset);

    //インデックスデータをバッファにコピー
    void* data = mpRef.MapMemory(stagingMem, stagingOffset, bufSize);
    memcpy(data, indices.data(), (size_t)bufSize);
    mpRef.UnmapMemory(stagingMem, stagingOffset, bufSize);;

    //メモリ転送操作で目的地にできるバッファを作成
    VkBuffer       indexBuf;
    VkDeviceMemory indexMem;
    VkDeviceSize   indexOffset = 0;
    CreateBuffer(bufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuf, indexMem, indexOffset);

    CopyBuffer(stagingBuf, indexBuf, bufSize);

    vkDestroyBuffer(device, stagingBuf, nullptr);
    mpRef.Free(stagingMem, stagingOffset, bufSize);

    return indexBuf;
}

void BufferResourceManager::CreateMVPUniformBuffers(size_t maxFramesInFlight)
{
    VkDeviceSize bufSize = sizeof(Utils::MVPUBO);

    MVPUBs.resize(maxFramesInFlight);
    MVPUBsMemory.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        VkDeviceSize offset = 0;
        CreateBuffer(bufSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            MVPUBs[i], MVPUBsMemory[i], offset);
    }

    rrRef.RegisterResource(RegistryKeys::MVPUBs, MVPUBs);
}

void BufferResourceManager::CreateStagingBuffer(VkDeviceSize size, 
    VkBuffer& stagingBuf, VkDeviceMemory& stagingMem, VkDeviceSize& stagingOffset)
{
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuf, stagingMem, stagingOffset);
}

void BufferResourceManager::CopyBufferToImage(VkBuffer buf, VkImage img, uint32_t width, uint32_t height) const
{
    auto commandBuf = Utils::BeginTransientCommands(device, commandPool);

    //バッファ内のピクセルをイメージのどこにコピーするかについて設定
    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuf, buf, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Utils::EndTransientCommands(device, graphicsQueue, commandPool, commandBuf);
}

void BufferResourceManager::UploadMVPUniformBuffer(uint32_t currentImg, const Utils::MVPUBO& ubo)
{
    VkDeviceSize bufSize = sizeof(ubo);
    VkDeviceSize offset = 0;

    void* data = mpRef.MapMemory(MVPUBsMemory[currentImg], offset, bufSize);
    memcpy(data, &ubo, bufSize);
    mpRef.UnmapMemory(MVPUBsMemory[currentImg],offset,bufSize);
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void BufferResourceManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer& buf, VkDeviceMemory& bufMem, VkDeviceSize& offset) const
{
    //生成バッファの詳細入力
    VkBufferCreateInfo bufInfo{};
    bufInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size        = size;
    bufInfo.usage       = usage;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    //バッファ生成
    VkResult result = vkCreateBuffer(device, &bufInfo, nullptr, &buf);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("BufferResourceManager: failed to create buffer!");
    }

    //生成バッファのメモリ割り当て
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buf, &memRequirements);
    bufMem = mpRef.Allocate(memRequirements.size, memRequirements.alignment, offset, properties);

    result = vkBindBufferMemory(device, buf, bufMem, offset);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("BufferResourceManager: failed to bind buffer memory!");
    }
}

void BufferResourceManager::CopyBuffer(VkBuffer srcBuf, VkBuffer dstBuf, VkDeviceSize size) const
{
    auto commandBuf = Utils::BeginTransientCommands(device, commandPool);

    //コピーするバイト数を設定
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuf, srcBuf, dstBuf, 1, &copyRegion);

    Utils::EndTransientCommands(device, graphicsQueue, commandPool, commandBuf);
}