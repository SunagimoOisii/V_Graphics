#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>

class ImageResourceManager
{
public:
    ImageResourceManager(class MemoryPool& mp,
        VkDevice d, VkPhysicalDevice pd, VkQueue graphicsQ, VkCommandPool cp);
    ~ImageResourceManager();

//***********************************************************
// ä÷êîÅ´
//***********************************************************

    std::vector<VkImageView>        CreateSwapChainImageViews(const std::vector<VkImage>& scImgs, VkFormat f);
    std::pair<VkImage, VkImageView> CreateColorAttachmentImage(VkExtent2D e, VkFormat f);
    std::pair<VkImage, VkImageView> CreateDepthAttachmentImage(VkExtent2D e);
    void CreateStagingImage(uint32_t width, uint32_t height, VkFormat f, VkImage& stagingImg, VkDeviceMemory& stagingImgMem);
    VkImageView CreateTextureImageView(VkImage textureImg, VkFormat f);

    void TransitionImageLayout(VkImage img, VkFormat f, VkImageLayout oldL, VkImageLayout newL) const;

private:
//***********************************************************
// ïœêîÅ´
//***********************************************************

    class MemoryPool& memoryPoolRef;

    VkDevice         device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;

    VkCommandPool commandPool;

//***********************************************************
// ä÷êîÅ´
//***********************************************************

    void CreateImage(uint32_t width, uint32_t height, VkFormat f,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& img, VkDeviceMemory& imgMem, VkDeviceSize& offset, VkDeviceSize& size) const;
    VkImageView CreateImageView(VkImage img, VkFormat f, VkImageAspectFlags aspectFlags) const;
};