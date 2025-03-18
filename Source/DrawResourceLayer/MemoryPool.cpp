#include <stdexcept>
#include <cstring>

#include "DrawResourceLayer/MemoryPool.h"
#include "Utilities.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

MemoryPool::MemoryPool(VkDevice d, VkPhysicalDevice pd, VkDeviceSize minBSize, VkDeviceSize maxBSize)
    : device(d)
    , physicalDevice(pd)
    , maxBlockSize(maxBSize)
    , minBlockSize(minBSize)
{
}

MemoryPool::~MemoryPool() 
{
    //各メモリタイプのブロックを解放
    for (auto& [typeIndex, blocks] : memoryPools)
    {
        for (auto& block : blocks)
        {
            std::cout << "MemoryPool: Attempting to free memory block: " << block.memory << std::endl;

            //フリーリストを解放
            MemoryBlock::FreeNode* currentNode = block.freeListHead;
            while (currentNode)
            {
                MemoryBlock::FreeNode* toDelete = currentNode;
                currentNode = currentNode->nextNode;
                delete toDelete;
            }

            //Vulkanメモリを解放
            vkFreeMemory(device, block.memory, nullptr);
            std::cout << "MemoryPool: Freed memory block: " << block.memory << std::endl;
        }
    }

    std::cout << "MemoryPool: All memory blocks have been freed." << std::endl;
}

VkDeviceMemory MemoryPool::Allocate(VkDeviceSize size, VkDeviceSize alignment,
    VkDeviceSize& offset, VkMemoryPropertyFlags properties)
{
    std::cout << "MemoryPool: AllocateInfo: Request size: " << size
        << ", Alignment: " << alignment
        << ", Creating new block" << std::endl;

    constexpr uint32_t ALL_MEMORY_TYPES = 0xFFFFFFFF;
    uint32_t memoryTypeIndex = Utils::FindMemoryType(physicalDevice, ALL_MEMORY_TYPES, properties);

    while (true)
    {
        //既存のブロックからメモリを割り当て
        for (auto& block : memoryPools[memoryTypeIndex])
        {
            MemoryBlock::FreeNode* prev = nullptr;
            MemoryBlock::FreeNode* currentNode = block.freeListHead;

            while (currentNode)
            {
                VkDeviceSize alignedOffset = (currentNode->offset + alignment - 1) & ~(alignment - 1);

                if (alignedOffset + size <= currentNode->offset + currentNode->size)
                {
                    offset = alignedOffset;

                    //フリーリストの更新
                    if (alignedOffset + size == currentNode->offset + currentNode->size)
                    {
                        if (prev)
                            prev->nextNode = currentNode->nextNode;
                        else
                            block.freeListHead = currentNode->nextNode;
                        delete currentNode;
                    }
                    else
                    {
                        currentNode->size -= (alignedOffset + size - currentNode->offset);
                        currentNode->offset = alignedOffset + size;
                    }

                    return block.memory;
                }

                prev = currentNode;
                currentNode = currentNode->nextNode;
            }
        }

        //リクエストサイズに基づいてブロックサイズを決定
        VkDeviceSize requiredBlockSize = std::max(size + alignment, minBlockSize);

        //新しいメモリブロックを作成
        VkDeviceMemory newBlock = CreateNewBlock(memoryTypeIndex, requiredBlockSize);
        memoryPools[memoryTypeIndex].push_back({ newBlock, requiredBlockSize, nullptr });

        auto& newBlockInfo = memoryPools[memoryTypeIndex].back();
        newBlockInfo.freeListHead = new MemoryBlock::FreeNode{ 0, requiredBlockSize, nullptr };

        std::cout << "MemoryPool: Created new block of size: " << requiredBlockSize << std::endl;
    }
}

void MemoryPool::Free(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size)
{
    for (auto& [typeIndex, blocks] : memoryPools)
    {
        for (auto& block : blocks)
        {
            if (block.memory != mem)
                continue;

            //マッピング範囲を削除
            for (auto it = block.mappedRanges.begin(); it != block.mappedRanges.end();)
            {
                if (it->first >= offset && it->first < offset + size)
                {
                    it = block.mappedRanges.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            //フリーリストに戻す
            auto newNode = new MemoryBlock::FreeNode{ offset, size, block.freeListHead };
            block.freeListHead = newNode;
            MergeFreeNodes(block);

            return;
        }
    }
    throw std::runtime_error("MemoryPool: Invalid memory block!");
}

void* MemoryPool::MapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size)
{
    for (auto& [typeIndex, blocks] : memoryPools)
    {
        for (auto& block : blocks)
        {
            if (block.memory == mem)
            {
                for (const auto& [mappedOffset, mappedData] : block.mappedRanges)
                {
                    if (offset < mappedOffset + size && mappedOffset < offset + size)
                    {
                        throw std::runtime_error("MemoryPool: Overlapping memory range detected!");
                    }
                }

                //新しい範囲をマッピング
                void* data;
                vkMapMemory(device, mem, offset, size, 0, &data);
                block.mappedRanges[offset] = data;
                return data;
            }
        }
    }
    throw std::runtime_error("MemoryPool: Memory block not found for mapping!");
}

void MemoryPool::UnmapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size)
{
    for (auto& [typeIndex, blocks] : memoryPools)
    {
        for (auto& block : blocks)
        {
            if (block.memory == mem)
            {
                auto it = block.mappedRanges.find(offset);
                if (it == block.mappedRanges.end())
                {
                    throw std::runtime_error("MemoryPool: Memory range is not mapped!");
                }

                //範囲をアンマップ
                block.mappedRanges.erase(it);
                vkUnmapMemory(device, mem);
                std::cout << "MemoryPool: Unmapped range at offset: " << offset 
                    << ", Size: " << size << std::endl;

                return;
            }
        }
    }
    throw std::runtime_error("MemoryPool: Memory block not found for unmapping!");
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

VkDeviceMemory MemoryPool::CreateNewBlock(uint32_t memTypeIndex, VkDeviceSize blockSize) const
{
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = blockSize;
    allocInfo.memoryTypeIndex = memTypeIndex;

    VkDeviceMemory mem;
    VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &mem);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("MemoryPool: Failed to allocate memory block!");
    }

    std::cout << "MemoryPool: Created new memory block of size: " << blockSize << std::endl;
    return mem;
}

void MemoryPool::PrintFreeList(const MemoryBlock& b) const
{
    std::cout << "MemoryPool: Free list for memory block: " << b.memory << std::endl;
    MemoryBlock::FreeNode* currentNode = b.freeListHead;
    while (currentNode)
    {
        std::cout << "  FreeNode - Offset: " << currentNode->offset
            << ", Size: " << currentNode->size << std::endl;
        currentNode = currentNode->nextNode;
    }
}

void MemoryPool::MergeFreeNodes(MemoryBlock& b) 
{
    if (!b.freeListHead) return;

    //フリーリストを結合
    MemoryBlock::FreeNode* currentNode = b.freeListHead;

    while (currentNode && 
        currentNode->nextNode)
    {
        //隣接ノードの結合
        if (currentNode->offset + currentNode->size == currentNode->nextNode->offset)
        {
            currentNode->size += currentNode->nextNode->size;

            //結合が完了したので、隣接ノードの存在を削除
            MemoryBlock::FreeNode* toDelete = currentNode->nextNode;
            currentNode->nextNode = currentNode->nextNode->nextNode;
            delete toDelete;
        }
        else
        {
            currentNode = currentNode->nextNode;
        }
    }
}