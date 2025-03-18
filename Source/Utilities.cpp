#include <stdexcept>
#include <fstream>
#include <iostream>

#include "Utilities.h"

namespace Utils
{
    uint32_t FindMemoryType(VkPhysicalDevice pd, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        //メモリのプロパティを検索
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(pd, &memProperties);

        //条件に合うメモリタイプを検索
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
        //候補のフォーマットからサポートされているものを探す
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
        //引数の物理デバイスで利用可能なスワップチェインの詳細をSurfaceCapabilities構造体に基づいて取得
        Utils::SurfaceCapabilities details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, sur, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pd, sur, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount); //利用可能formatを全て保持できるようにする
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
        //一時的なコマンドバッファの割り当て設定
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cp;
        allocInfo.commandBufferCount = 1;

        //コマンドバッファ割り当て
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

        //コマンドバッファの記録を開始
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndTransientCommands(VkDevice d, VkQueue graphicsQ, VkCommandPool cp, VkCommandBuffer cb)
    {
        //一時的なコマンドバッファの記録を終了
        vkEndCommandBuffer(cb);

        //コマンドバッファをキューに提出
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pCommandBuffers    = &cb;
        submitInfo.commandBufferCount = 1;

        vkQueueSubmit(graphicsQ, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQ);

        //一時的なコマンドバッファを解放
        vkFreeCommandBuffers(d, cp, 1, &cb);
        std::cout << "Utils: Free TransientCommandBuffer" << std::endl;
    }

    std::vector<char> ReadFile(const std::string& fileName)
    {
        //読込位置をファイル末尾にしているのはその位置でファイルサイズを決定しバッファ確保を行うため。
        //バイナリモードなのはSPIR-Vに合わせるため。
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open File!: FileName:" + fileName);
        }
        else
        {
            std::cout << "Opened sprFile: FileName: " + fileName << std::endl;
        }

        //読込位置(終点)までのサイズでバッファ作成
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        //ファイル末尾までの文字量でストリームからバッファに書込(このときbufferの末尾はヌル文字ではない)
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
}