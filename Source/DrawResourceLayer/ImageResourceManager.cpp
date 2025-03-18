#include <stdexcept>
#include <iostream>

#include "DrawResourceLayer/ImageResourceManager.h"
#include "DrawResourceLayer/MemoryPool.h"

#include "Utilities.h"

//************************************************************************************************************************
// public��
//************************************************************************************************************************

ImageResourceManager::ImageResourceManager(MemoryPool& mp,
    VkDevice d, VkPhysicalDevice pd, VkQueue graphicsQ, VkCommandPool cp)
    : commandPool(cp)
    , device(d)
    , graphicsQueue(graphicsQ)
    , memoryPoolRef(mp)
    , physicalDevice(pd)
{
}

ImageResourceManager::~ImageResourceManager()
{
}

std::vector<VkImageView> ImageResourceManager::CreateSwapChainImageViews(const std::vector<VkImage>& scImgs, VkFormat f)
{
    std::vector<VkImageView> scImgViews(scImgs.size());

    for (size_t i = 0; i < scImgs.size(); ++i)
    {
        scImgViews[i] = CreateImageView(scImgs[i], f, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    return scImgViews;
}

std::pair<VkImage, VkImageView> ImageResourceManager::CreateColorAttachmentImage(VkExtent2D e, VkFormat f)
{
    VkImage        img;
    VkDeviceMemory mem;
    VkDeviceSize   offset;
    VkDeviceSize   size;

    CreateImage(e.width, e.height, f, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        img, mem, offset, size);

    VkImageView imgView = CreateImageView(img, f, VK_IMAGE_ASPECT_COLOR_BIT);

    return { img, imgView };
}

std::pair<VkImage, VkImageView> ImageResourceManager::CreateDepthAttachmentImage(VkExtent2D e)
{
    VkImage        img;
    VkDeviceMemory mem;
    VkDeviceSize   offset;
    VkDeviceSize   size;

    VkFormat depthF = Utils::FindDepthFormat(physicalDevice);

    CreateImage(e.width, e.height, depthF, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        img, mem, offset, size);

    VkImageView imgView = CreateImageView(img, depthF, VK_IMAGE_ASPECT_DEPTH_BIT);

    return { img, imgView };
}

void ImageResourceManager::CreateStagingImage(uint32_t width, uint32_t height, VkFormat f, 
    VkImage& stagingImg, VkDeviceMemory& stagingImgMem)
{
    VkDeviceSize offset;
    VkDeviceSize size;

    CreateImage(width, height, f, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        stagingImg, stagingImgMem, offset, size);
}

VkImageView ImageResourceManager::CreateTextureImageView(VkImage textureImg, VkFormat f)
{
    return CreateImageView(textureImg, f, VK_IMAGE_ASPECT_COLOR_BIT);
}

void ImageResourceManager::TransitionImageLayout(VkImage image, VkFormat f, VkImageLayout oldL, VkImageLayout newL) const
{
    auto commandBuf = Utils::BeginTransientCommands(device, commandPool);

    //�o���A�𐶐������C�A�E�g�J�ڂɈ��萫����������
    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldL;
    barrier.newLayout = newL;
    //�L���[�t�@�~���[�̏��L���]���Ńo���A���g�p����Ƃ��ȊO��IGNORED�ɂ���
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //�e�����󂯂�摜�̎w��
    barrier.image = image;

    //�C���[�W���ǂ̂悤�ȃA�^�b�`�����g�Ƃ��Ďg�p�����̂��̌���
    if (newL == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (f == VK_FORMAT_D32_SFLOAT_S8_UINT || 
            f == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldL == VK_IMAGE_LAYOUT_UNDEFINED &&
        newL == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldL == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newL == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("ImageResourceManager: Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuf, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    Utils::EndTransientCommands(device, graphicsQueue, commandPool, commandBuf);
}

//************************************************************************************************************************
// private��
//************************************************************************************************************************

void ImageResourceManager::CreateImage(uint32_t width, uint32_t height, VkFormat f, 
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkImage& img, VkDeviceMemory& imgMem, VkDeviceSize& offset, VkDeviceSize& size) const
{
    // �C���[�W�̐ݒ�
    VkImageCreateInfo imgInfo{};
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.extent.width  = width;
    imgInfo.extent.height = height;
    imgInfo.extent.depth  = 1;
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = 1;
    imgInfo.format        = f;
    imgInfo.tiling        = tiling;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage         = usage;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    //�C���[�W�쐬
    VkResult result = vkCreateImage(device, &imgInfo, nullptr, &img);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    //�������v���擾
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, img, &memRequirements);

    //MemoryPool���烁�������蓖��
    imgMem = memoryPoolRef.Allocate(memRequirements.size, memRequirements.alignment, offset, properties);
    size   = memRequirements.size;

    vkBindImageMemory(device, img, imgMem, offset);
}

VkImageView ImageResourceManager::CreateImageView(VkImage img, VkFormat f, VkImageAspectFlags aspectFlags) const
{
    //��������C���[�W�r���[�̐ݒ�
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = img;
    //�X���b�v�`�F�C�����̉摜�f�[�^�̉��ߕ��@
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = f;
    //�J���[�`�����l��
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //���̑��摜�f�[�^�������\�[�X�ɂ���
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    //�C���[�W�r���[����
    VkImageView imgView;
    VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imgView);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("ImageResourceManager: failed to create image view!");
    }

    return imgView;
}