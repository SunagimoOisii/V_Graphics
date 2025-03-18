#include "HardwareAbstractionLayer/HardwareAbstractionLayer.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// publicÅ´
//************************************************************************************************************************

HardwareAbstractionLayer::HardwareAbstractionLayer(ResourceRegistry& rrRef)
{
    //deviceManager
    auto ins = rrRef.FindResource<VkInstance>(RegistryKeys::VkInstance);
    auto sur = rrRef.FindResource<VkSurfaceKHR>(RegistryKeys::Surface);
    deviceManager = std::make_unique<DeviceManager>(rrRef, ins, sur);

    //commandResourceManager
    auto maxFrames      = rrRef.FindResource<int>(RegistryKeys::MaxFramesInFlight);
    auto dev            = rrRef.FindResource<VkDevice>(RegistryKeys::Device);
    auto graphicsQ      = rrRef.FindResource<VkQueue>(RegistryKeys::GraphicsQueue);
    auto qFamilyIndices = rrRef.FindResource<Utils::QueueFamilyIndices>(RegistryKeys::QueueFamilyIndices);
    commandResourceManager = std::make_unique<CommandResourceManager>(
        rrRef, maxFrames, dev, graphicsQ, qFamilyIndices);
    commandResourceManager->CreateCommandPool();
    commandResourceManager->CreateCommandBuffers();
}

HardwareAbstractionLayer::~HardwareAbstractionLayer()
{
    commandResourceManager.reset();
    deviceManager.reset();
}