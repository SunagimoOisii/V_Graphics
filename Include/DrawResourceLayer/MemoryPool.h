#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include <iostream>

/// <summary>
/// VkDeviceMemory���蓖�Ă��œK������N���X<para></para>
/// �t���[���X�g�𗘗p���ď����ȃ��������蓖�Ă��������A
/// �u���b�N�����A������ʂ��ăt���O�����e�[�V�������ŏ�������
/// </summary>
class MemoryPool 
{
public:
    MemoryPool(VkDevice d, VkPhysicalDevice pd, VkDeviceSize minBSize, VkDeviceSize maxBSize);
    ~MemoryPool();

//***********************************************************
// �֐���
//***********************************************************

    VkDeviceMemory Allocate(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize& offset, VkMemoryPropertyFlags properties);
    void           Free(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);

    void* MapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);
    void  UnmapMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size);

private:
    /// <summary>
    /// �P���VkDeviceMemory�u���b�N���Ǘ�����<para></para>
    /// �t���[���X�g��p���ċ󂫗̈��ǐՂ���
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

        //offset���Ƃ̃}�b�s���O�f�[�^���Ǘ�
        std::unordered_map<VkDeviceSize, void*> mappedRanges;

        bool isMapped = false;
    };

//***********************************************************
// �ϐ���
//***********************************************************

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkDeviceSize minBlockSize;
    VkDeviceSize maxBlockSize;

    //�������^�C�v���Ƃ̃u���b�N�Ǘ����s��
    std::unordered_map<uint32_t, std::vector<MemoryBlock>> memoryPools;

//***********************************************************
// �֐���
//***********************************************************

    VkDeviceMemory CreateNewBlock(uint32_t memTypeIndex, VkDeviceSize blockSize) const;

    /// <summary>
    /// �אڂ���t���[���X�g�m�[�h���������t���O�����e�[�V������h�~����<para></para>
    /// ������A�אڃm�[�h�͍폜�����
    /// </summary>
    void MergeFreeNodes(MemoryBlock& b);

    //�t���[���X�g���f�o�b�O�o��
    void PrintFreeList(const MemoryBlock& b) const; 
};