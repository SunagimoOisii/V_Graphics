#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include <iostream>

/// <summary>
/// VkDeviceMemory割り当てを最適化するクラス<para></para>
/// フリーリストを利用して小さなメモリ割り当てを効率化、
/// ブロック分割、統合を通じてフラグメンテーションを最小化する
/// </summary>
class MemoryPool 
{
public:
    MemoryPool(VkDevice d, VkPhysicalDevice pd, VkDeviceSize minBSize, VkDeviceSize maxBSize);
    ~MemoryPool();

//***********************************************************
// 関数↓
//***********************************************************

    VkDeviceMemory Allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& offset, VkMemoryPropertyFlags properties);
    void           Free(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);

    void* MapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    void  UnmapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);

private:
    /// <summary>
    /// 単一のVkDeviceMemoryブロックを管理する<para></para>
    /// フリーリストを用いて空き領域を追跡する
    /// </summary>
    struct MemoryBlock 
    {
        VkDeviceMemory memory;
        VkDeviceSize size;

        struct FreeNode 
        {
            VkDeviceSize offset;
            VkDeviceSize size;
            FreeNode* nextNode;
        };
        FreeNode* freeListHead;

        //offsetごとのマッピングデータを管理
        std::unordered_map<VkDeviceSize, void*> mappedRanges;

        bool isMapped = false;
    };

//***********************************************************
// 変数↓
//***********************************************************

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkDeviceSize minBlockSize;
    VkDeviceSize maxBlockSize;

    //メモリタイプごとのブロック管理を行う
    std::unordered_map<uint32_t, std::vector<MemoryBlock>> memoryPools;

//***********************************************************
// 関数↓
//***********************************************************

    VkDeviceMemory CreateNewBlock(uint32_t memTypeIndex, VkDeviceSize blockSize) const;

    /// <summary>
    /// 隣接するフリーリストノードを結合しフラグメンテーションを防止する<para></para>
    /// 結合後、隣接ノードは削除される
    /// </summary>
    void MergeFreeNodes(MemoryBlock& b);

    //フリーリストをデバッグ出力
    void PrintFreeList(const MemoryBlock& b) const; 
};