#include "RenderingLayer/RenderingLayer.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// publicÅ´
//************************************************************************************************************************

RenderingLayer::RenderingLayer(ResourceRegistry& rrRef,Camera& cam,
    VkBuffer vertexB, VkBuffer indexB, const std::vector<uint16_t> indices,
    std::function<VkImageView(std::string)> getTextureImageViewF,
    std::function<VkSampler(std::string)> getTextureSamplerF,
    std::function<std::vector<VkImageView>(const std::vector<VkImage>&, VkFormat)> createSwapChainImageViewsF,
    std::function<std::pair<VkImage, VkImageView>(VkExtent2D)> createDepthAttachmentImageF,
    std::function<void(uint32_t, const Utils::MVPUBO&)> uploadMVPUniformBuffersF)
{
    //äeÉNÉâÉXÇÃèâä˙âª
    auto maxFrames      = rrRef.FindResource<int>(RegistryKeys::MaxFramesInFlight);
    auto dev            = rrRef.FindResource<VkDevice>(RegistryKeys::Device);
    auto pDev           = rrRef.FindResource<VkPhysicalDevice>(RegistryKeys::PhysicalDevice);
    auto graphicsQ      = rrRef.FindResource<VkQueue>(RegistryKeys::GraphicsQueue);
    auto qFamilyIndices = rrRef.FindResource<Utils::QueueFamilyIndices>(RegistryKeys::QueueFamilyIndices);

    descriptorManager = std::make_unique<DescriptorManager>(
        rrRef, dev, maxFrames,
        getTextureImageViewF, getTextureSamplerF);
    descriptorManager->CreateDescriptorSetLayout();
    descriptorManager->CreateDescriptorPool();
    descriptorManager->CreateDescriptorSets();

    swapChainManager = std::make_unique<SwapChainManager>(
        rrRef, dev, pDev, qFamilyIndices,
        createSwapChainImageViewsF, createDepthAttachmentImageF);

    renderPassManager = std::make_unique<RenderPassManager>(
        rrRef, dev, pDev,
        [&]() { return swapChainManager->GetSwapChainImageFormat(); });

    auto dSetLayout = rrRef.FindResource<VkDescriptorSetLayout>(RegistryKeys::DescriptorSetLayout);
    graphicsPipelineManager = std::make_unique<GraphicsPipelineManager>(
        rrRef, dev, dSetLayout);

    frameBufferManager = std::make_unique<FrameBufferManager>(
        rrRef, dev,
        [&]() { return swapChainManager->GetSwapChainImageViews(); },
        [&]() { return swapChainManager->GetSwapChainDepthImageView(); });

    renderSyncManager = std::make_unique<RenderSyncManager>(dev, maxFrames);
    
    CreateRenderingFlow("Shaders/vert.spv", "Shaders/frag.spv");

    renderer = std::make_unique<Renderer>(rrRef, dev, cam, maxFrames,
        vertexB, indexB, indices,
        renderSyncManager.get(),
        uploadMVPUniformBuffersF,
        [&]() {return frameBufferManager->GetFrameBuffers();},
        [&]() {RecreateRenderingFlow();});
}

RenderingLayer::~RenderingLayer()
{
    renderer.reset();
    renderSyncManager.reset();
    frameBufferManager.reset();
    graphicsPipelineManager.reset();
    renderPassManager.reset();
    swapChainManager.reset();
    descriptorManager.reset();
}

//***********************************************************
// ä÷êîÅ´
//***********************************************************

void RenderingLayer::Render(bool& loopFlag, float deltaTime)
{
    renderer->Render(loopFlag, deltaTime);
}

void RenderingLayer::CreateRenderingFlow(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    swapChainManager->CreateSwapChain();
    renderPassManager->CreateRenderPass();
    graphicsPipelineManager->CreateGraphicsPipeline(vertexShaderPath, fragmentShaderPath);
    frameBufferManager->CreateFrameBuffers();

    renderSyncManager->CreateSyncObjects();
}

void RenderingLayer::RecreateRenderingFlow()
{
    swapChainManager->RecreateSwapChain();
    frameBufferManager->CreateFrameBuffers();
}