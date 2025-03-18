#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <functional>

#include "Vertex.h"
#include "Utilities.h"

/// <summary>
/// �o�b�t�@���\�[�X�𐶐��E�Ǘ�����N���X<para></para>
/// ���_,�C���f�b�N�X�o�b�t�@�̐������s���A�����̏��L�͌Ăяo�����Ɉς˂�<para></para>
/// �܂��A���j�t�H�[���o�b�t�@�Ɋւ��Ă͐����E���L�E�X�V���ꊇ�Ǘ����A
/// �t���[�����Ƃ̃f�[�^�]�����T�|�[�g����
/// </summary>
class BufferResourceManager
{
public:
    BufferResourceManager(VkDevice d, VkPhysicalDevice pd, VkQueue graphicsQ,
        VkCommandPool cp, class MemoryPool& mpRef, class ResourceRegistry& rrRef);
    ~BufferResourceManager();

//***********************************************************
// �֐���
//***********************************************************

    VkBuffer CreateVertexBuffer(const std::vector<Vertex>& vertices);
    VkBuffer CreateIndexBuffer(const std::vector<uint16_t>& indices);
    void CreateStagingBuffer(VkDeviceSize size, VkBuffer& stagingBuf, VkDeviceMemory& stagingMem, VkDeviceSize& stagingOffset);

    void CopyBufferToImage(VkBuffer buf, VkImage img, uint32_t width, uint32_t height) const;

    void CreateMVPUniformBuffers(size_t maxFramesInFlight);
    void UploadMVPUniformBuffer(uint32_t currentImg, const Utils::MVPUBO& ubo);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkDevice         device;
    VkPhysicalDevice physicalDevice;
    VkQueue          graphicsQueue;

    VkCommandPool commandPool;

    class MemoryPool&       mpRef;
    class ResourceRegistry& rrRef;

//****************************
// UniformBuffer(UB)�ϐ���
//   ��r�I�f�[�^��������,���t���[���X�V�����̂ŃX�e�[�W���O�o�b�t�@���쐬���Ȃ�
//****************************

    std::vector<VkBuffer>       MVPUBs; 
    std::vector<VkDeviceMemory> MVPUBsMemory;

//***********************************************************
// �֐���
//***********************************************************

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buf, VkDeviceMemory& bufMem, VkDeviceSize& offset) const;

    /// <summary>
    /// �V�F�[�_�̃o�b�t�@�ɍœK�ȃ������ɂ�VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT������,�����CPU����ł̓A�N�Z�X�ł��Ȃ��\��������
    /// �����vkMapMemory��, GPU�������̈��CPU���ŗp���邽�߂ɃA�N�Z�X�\�ɂ��邪, ���̋t�͂ł��Ȃ�����,
    /// ���̊֐��ŃX�e�[�W���O�o�b�t�@����e�V�F�[�_�̃o�b�t�@(GPU��)�ւ̃R�s�[���s��GPU���ŃV�F�[�_�f�[�^��������悤�ɂ���
    /// </summary>
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
};