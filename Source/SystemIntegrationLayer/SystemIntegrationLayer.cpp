#include "SystemIntegrationLayer/SystemIntegrationLayer.h"
#include "ResourceRegistry.h"
#include "Utilities.h"

SystemIntegrationLayer::SystemIntegrationLayer(ResourceRegistry& rrRef,
    bool enableValidationLayers,
    uint32_t width, uint32_t height, const char* windowTitle)
{
    //vulkanInitializeManager‰Šú‰»
    vulkanInitializeManager = std::make_unique<VulkanInitializeManager>(enableValidationLayers, rrRef);
    vulkanInitializeManager->Initialize();

    //windowManager‰Šú‰»
    auto instance = rrRef.FindResource<VkInstance>(RegistryKeys::VkInstance);
    windowManager = std::make_unique<WindowManager>(instance, 
        width, height, windowTitle,
        rrRef);
}

SystemIntegrationLayer::~SystemIntegrationLayer()
{
    windowManager.reset();
    vulkanInitializeManager.reset();
}