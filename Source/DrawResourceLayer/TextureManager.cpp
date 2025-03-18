#include <stb_image.h>

#include "DrawResourceLayer/TextureManager.h"
#include "DrawResourceLayer/MemoryPool.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

TextureManager::TextureManager(class MemoryPool& mp,
    VkDevice d, BufferResourceManager* brManager, ImageResourceManager* irManager)
    : bufferResourceManager(brManager)
    , device(d)
    , imageResourceManager(irManager)
    , memoryPoolRef(mp)
{
}

TextureManager::~TextureManager()
{
    for (auto& texturePair : textures)
    {
        const std::string& textureName = texturePair.first;
        TextureData& texture = texturePair.second;

        if (texture.sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, texture.sampler, nullptr);
            std::cout << "TextureManager: Destroyed sampler for texture: " << textureName << std::endl;
        }
        if (texture.imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, texture.imageView, nullptr);
            std::cout << "TextureManager: Destroyed image view for texture: " << textureName << std::endl;
        }
        if (texture.image != VK_NULL_HANDLE)
        {
            vkDestroyImage(device, texture.image, nullptr);
            std::cout << "TextureManager: Destroyed image for texture: " << textureName << std::endl;
        }
    }

    std::cout << "TextureManager: All textures have been destroyed." << std::endl;
}

VkImageView TextureManager::GetTextureImageView(const std::string& textureName)
{
    if (textures.find(textureName) == textures.end())
    {
        throw std::runtime_error("TextureManager: Texture not found: " + textureName);
    }
    return textures[textureName].imageView;
}

VkSampler TextureManager::GetTextureSampler(const std::string& textureName)
{
    if (textures.find(textureName) == textures.end())
    {
        throw std::runtime_error("TextureManager: Texture not found: " + textureName);
    }
    return textures[textureName].sampler;
}

void TextureManager::LoadTexture(const std::string& textureName, const std::string& filePath)
{
    if (textures.find(textureName) != textures.end())
    {
        throw std::runtime_error("TextureManager: Texture already loaded: " + textureName);
    }

    TextureData textureData;
    CreateTextureImage(filePath, textureData);
    CreateTextureImageView(textureData);
    CreateTextureSampler(textureData);

    textures[textureName] = textureData;
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void TextureManager::CreateTextureImage(const std::string& filePath, TextureData& textureData)
{
    //画像読み込み
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
    {
        throw std::runtime_error("TextureManager: Failed to load texture image from file: " + filePath);
    }

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

    //ステージングバッファ作成
    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingMemory;
    VkDeviceSize   stagingOffset = 0;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = imageSize;
    bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("TextureManager: Failed to create staging buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    stagingMemory = memoryPoolRef.Allocate(memRequirements.size, memRequirements.alignment, stagingOffset,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(device, stagingBuffer, stagingMemory, stagingOffset);

    //ピクセル値をステージングバッファにコピー
    void* data = memoryPoolRef.MapMemory(stagingMemory, stagingOffset, imageSize);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    memoryPoolRef.UnmapMemory(stagingMemory, stagingOffset, imageSize);

    stbi_image_free(pixels);

    //テクスチャイメージ生成
    imageResourceManager->CreateStagingImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, textureData.image, textureData.memory);

    //ステージングバッファをテクスチャイメージにコピー
    imageResourceManager->TransitionImageLayout(textureData.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    bufferResourceManager->CopyBufferToImage(stagingBuffer, textureData.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    imageResourceManager->TransitionImageLayout(textureData.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    //ステージングバッファの解放
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    memoryPoolRef.Free(stagingMemory, stagingOffset, imageSize);
}


void TextureManager::CreateTextureImageView(TextureData& textureData)
{
    textureData.imageView = imageResourceManager->CreateTextureImageView(textureData.image, VK_FORMAT_R8G8B8A8_SRGB);
}

void TextureManager::CreateTextureSampler(TextureData& textureData) const
{
    //生成サンプラーの詳細入力
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //拡大,縮小されたテクセルの補間方法
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //テクスチャの寸法を超えた際の軸ごとの挙動
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    //adressModeでClampToBorderを選択した際使用する色
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    //異方性フィルタリング
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy    = 16;
    //テクスチャのテクセルのアドレスに非正規化座標を使用するか選択
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;
    //ミップマッピングに関する設定
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod     = 0.0f;
    samplerInfo.maxLod     = 0.0f;

    VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &textureData.sampler);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("TextureManager: Failed to create texture sampler!");
    }
}