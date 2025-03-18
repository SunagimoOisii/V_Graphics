#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <stdexcept>

#include "BufferResourceManager.h"
#include "ImageResourceManager.h"

class TextureManager
{
public:
    TextureManager(class MemoryPool& mp,
        VkDevice d, BufferResourceManager* brManager, ImageResourceManager* irManager);
    ~TextureManager();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    VkImageView GetTextureImageView(const std::string& textureName);
    VkSampler   GetTextureSampler(const std::string& textureName);

//***********************************************************
// �֐���
//***********************************************************

    void LoadTexture(const std::string& textureName, const std::string& filePath);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    class MemoryPool& memoryPoolRef;

    BufferResourceManager* bufferResourceManager;
    ImageResourceManager*  imageResourceManager;

    VkDevice device;

    struct TextureData
    {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;
    };
    std::unordered_map<std::string, TextureData> textures;

//***********************************************************
// �֐���
//***********************************************************

    void CreateTextureImage(const std::string& filePath, TextureData& textureData);
    void CreateTextureImageView(TextureData& textureData);
    void CreateTextureSampler(TextureData& textureData) const;
};