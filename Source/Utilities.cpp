#include <stdexcept>
#include <fstream>
#include <iostream>

#include "Utilities.h"

namespace Utils
{
    uint32_t FindMemoryType(VkPhysicalDevice pd, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        //�������̃v���p�e�B������
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(pd, &memProperties);

        //�����ɍ����������^�C�v������
        for (uint16_t i = 0;i < memProperties.memoryTypeCount;i++)
        {
            if (typeFilter & (1 << i) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Utils: failed to Find suitable MemoryType!");
    }

    VkFormat FindSupportedFormat(VkPhysicalDevice pd, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        //���̃t�H�[�}�b�g����T�|�[�g����Ă�����̂�T��
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(pd, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("Utils: failed to Find SupportedFormat!");
    }

    VkFormat FindDepthFormat(VkPhysicalDevice pd)
    {
        return FindSupportedFormat
        (
            pd, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    Utils::SurfaceCapabilities QuerySurfaceSupport(VkPhysicalDevice pd, VkSurfaceKHR sur)
    {
        //�����̕����f�o�C�X�ŗ��p�\�ȃX���b�v�`�F�C���̏ڍׂ�SurfaceCapabilities�\���̂Ɋ�Â��Ď擾
        Utils::SurfaceCapabilities details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, sur, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pd, sur, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount); //���p�\format��S�ĕێ��ł���悤�ɂ���
            vkGetPhysicalDeviceSurfaceFormatsKHR(pd, sur, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pd, sur, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pd, sur, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkCommandBuffer BeginTransientCommands(VkDevice d, VkCommandPool cp)
    {
        //�ꎞ�I�ȃR�}���h�o�b�t�@�̊��蓖�Đݒ�
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cp;
        allocInfo.commandBufferCount = 1;

        //�R�}���h�o�b�t�@���蓖��
        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(d, &allocInfo, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Utils: Failed to allocate CommandBuffer!");
        }
        else
        {
            std::cout << "Utils: Allocated TransientCommandBuffer" << std::endl;
        }

        //�R�}���h�o�b�t�@�̋L�^���J�n
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndTransientCommands(VkDevice d, VkQueue graphicsQ, VkCommandPool cp, VkCommandBuffer cb)
    {
        //�ꎞ�I�ȃR�}���h�o�b�t�@�̋L�^���I��
        vkEndCommandBuffer(cb);

        //�R�}���h�o�b�t�@���L���[�ɒ�o
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pCommandBuffers    = &cb;
        submitInfo.commandBufferCount = 1;

        vkQueueSubmit(graphicsQ, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQ);

        //�ꎞ�I�ȃR�}���h�o�b�t�@�����
        vkFreeCommandBuffers(d, cp, 1, &cb);
        std::cout << "Utils: Free TransientCommandBuffer" << std::endl;
    }

    std::vector<char> ReadFile(const std::string& fileName)
    {
        //�Ǎ��ʒu���t�@�C�������ɂ��Ă���̂͂��̈ʒu�Ńt�@�C���T�C�Y�����肵�o�b�t�@�m�ۂ��s�����߁B
        //�o�C�i�����[�h�Ȃ̂�SPIR-V�ɍ��킹�邽�߁B
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open File!: FileName:" + fileName);
        }
        else
        {
            std::cout << "Opened sprFile: FileName: " + fileName << std::endl;
        }

        //�Ǎ��ʒu(�I�_)�܂ł̃T�C�Y�Ńo�b�t�@�쐬
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        //�t�@�C�������܂ł̕����ʂŃX�g���[������o�b�t�@�ɏ���(���̂Ƃ�buffer�̖����̓k�������ł͂Ȃ�)
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
}