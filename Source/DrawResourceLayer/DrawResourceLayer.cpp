#include <chrono>

#include "DrawResourceLayer/DrawResourceLayer.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

DrawResourceLayer::DrawResourceLayer(ResourceRegistry& rrRef)
{
    //各クラス初期化に必要なリソースの取得
    auto dev       = rrRef.FindResource<VkDevice>(RegistryKeys::Device);
    auto pDev      = rrRef.FindResource<VkPhysicalDevice>(RegistryKeys::PhysicalDevice);
    auto graphicsQ = rrRef.FindResource<VkQueue>(RegistryKeys::GraphicsQueue);
    auto cp        = rrRef.FindResource<VkCommandPool>(RegistryKeys::CommandPool);

    //各クラスの初期化
    memoryPool = std::make_unique<MemoryPool>(dev, pDev, 256, 64 * 1024 * 1024);
    bufferResourceManager = std::make_unique<BufferResourceManager>(dev, pDev, graphicsQ, cp,
        *memoryPool.get(), rrRef);
    imageResourceManager  = std::make_unique<ImageResourceManager>(*memoryPool.get(), dev, pDev, graphicsQ, cp);
    textureManager        = std::make_unique<TextureManager>(*memoryPool.get(), dev,
        bufferResourceManager.get(), imageResourceManager.get());
}

DrawResourceLayer::~DrawResourceLayer() 
{
    //MemoryPoolはリソース生成マネージャークラスより後に破棄する
    textureManager.reset();
    bufferResourceManager.reset();
    imageResourceManager.reset();
    
    memoryPool.reset();
}

//***********************************************************
// ゲッターセッター↓
//***********************************************************

VkImageView DrawResourceLayer::GetTextureImageView(const std::string& textureName)
{
    return textureManager->GetTextureImageView(textureName);
}

VkSampler DrawResourceLayer::GetTextureSampler(const std::string& textureName)
{
    return textureManager->GetTextureSampler(textureName);
}

//***********************************************************
// 関数↓
//***********************************************************

//****************************
// バッファ(VkBuffer)系↓
//****************************

VkBuffer DrawResourceLayer::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
    return bufferResourceManager->CreateVertexBuffer(vertices);
}

VkBuffer DrawResourceLayer::CreateIndexBuffer(const std::vector<uint16_t>& indices)
{
    return bufferResourceManager->CreateIndexBuffer(indices);
}

void DrawResourceLayer::CreateMVPUniformBuffers(size_t maxFramesInFlight)
{
    bufferResourceManager->CreateMVPUniformBuffers(maxFramesInFlight);
}

void DrawResourceLayer::UploadMVPUniformBuffers(uint32_t currentImage, const Utils::MVPUBO& ubo)
{
    bufferResourceManager->UploadMVPUniformBuffer(currentImage, ubo);
}

//****************************
// バッファ(VkImage)系↓
//****************************

std::vector<VkImageView> DrawResourceLayer::CreateSwapChainImageViews(const std::vector<VkImage>& scImages, VkFormat format)
{
    return imageResourceManager->CreateSwapChainImageViews(scImages, format);
}

std::pair<VkImage, VkImageView> DrawResourceLayer::CreateColorAttachmentImage(VkExtent2D extent, VkFormat format)
{
    return imageResourceManager->CreateColorAttachmentImage(extent, format);
}

std::pair<VkImage, VkImageView> DrawResourceLayer::CreateDepthAttachmentImage(VkExtent2D extent)
{
    return imageResourceManager->CreateDepthAttachmentImage(extent);
}

//****************************
// テクスチャ系↓
//****************************

void DrawResourceLayer::LoadTexture(const std::string& textureName, const std::string& filePath)
{
    textureManager->LoadTexture(textureName, filePath);
}