#pragma once
#include <memory>

#include "MemoryPool.h"

#include "BufferResourceManager.h"
#include "ImageResourceManager.h"
#include "TextureManager.h"

/// <summary>
/// 描画に必要なリソースを統括管理するレイヤーラップクラス<para></para>
/// 頂点,インデックスバッファ、テクスチャ、イメージなどのリソース生成および管理を担い、<para></para>
/// 描画処理に必要な各種を提供する役割を担う
/// </summary>
class DrawResourceLayer
{
public:
    DrawResourceLayer(class ResourceRegistry& rrRef);
    ~DrawResourceLayer();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    VkImageView GetTextureImageView(const std::string& textureName);
    VkSampler   GetTextureSampler(const std::string& textureName);

//***********************************************************
// 関数↓
//***********************************************************

//****************************
// バッファ(VkBuffer)系↓
//****************************

    VkBuffer CreateVertexBuffer(const std::vector<Vertex>& vertices);
    VkBuffer CreateIndexBuffer(const std::vector<uint16_t>& indices);
    void     CreateMVPUniformBuffers(size_t maxFramesInFlight);
    void     UploadMVPUniformBuffers(uint32_t currentImage, const Utils::MVPUBO& ubo);

//****************************
// バッファ(VkImage)系↓
//****************************

    std::vector<VkImageView> CreateSwapChainImageViews(const std::vector<VkImage>& scImages, VkFormat format);

    std::pair<VkImage, VkImageView> CreateColorAttachmentImage(VkExtent2D extent, VkFormat format);
    std::pair<VkImage, VkImageView> CreateDepthAttachmentImage(VkExtent2D extent);

//****************************
// テクスチャ系↓
//****************************

    void LoadTexture(const std::string& textureName, const std::string& filePath);

private:
//***********************************************************
// 変数↓
//***********************************************************

    std::unique_ptr<MemoryPool> memoryPool;

    std::unique_ptr<BufferResourceManager> bufferResourceManager;
    std::unique_ptr<ImageResourceManager>  imageResourceManager;
    std::unique_ptr<TextureManager>        textureManager;
};