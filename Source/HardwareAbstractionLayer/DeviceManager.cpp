#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>
#include <string>

#include "HardwareAbstractionLayer/DeviceManager.h"
#include "ResourceRegistry.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

DeviceManager::DeviceManager(ResourceRegistry& rrRef,
    VkInstance ins, VkSurfaceKHR sur)
    : instance(ins)
    , surface(sur)
    , rrRef(rrRef)
{
    if (ins == VK_NULL_HANDLE)
    {
        throw std::runtime_error("DeviceManager: Invalid Vulkan instance provided to DeviceManager");
    }
    if (sur == VK_NULL_HANDLE)
    {
        throw std::runtime_error("DeviceManager: Invalid surface provided to DeviceManager");
    }

    PickPhysicalDevice();
    CreateLogicalDevice();
}

DeviceManager::~DeviceManager()
{
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
        std::cout << "DeviceManager: Logical device destroyed." << std::endl;
    }
    if (surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
        std::cout << "DeviceManager: Surface destroyed." << std::endl;
    }
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

bool DeviceManager::IsDeviceSuitable(VkPhysicalDevice pDev)
{
    //キューファミリーを検索し、拡張機能のサポート状態を取得する。
    //なおキューファミリーはSwapChain生成時にも必要となるため、
    //ResourceRegistryに登録しておく
    indices = FindQueueFamilies(pDev);
    rrRef.RegisterResource(RegistryKeys::QueueFamilyIndices, indices);
    bool extensionsSupported = CheckDeviceExtensionSupport(pDev);

    //必要な機能を持っているか確認
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(pDev, &supportedFeatures);
    if (!indices.IsComplete()) 
    {
        std::cout << "DeviceManager: Required queue families are not available." << std::endl;
    }
    if (!extensionsSupported) 
    {
        std::cout << "DeviceManager: Required device extensions are not supported." << std::endl;
    }
    if (!supportedFeatures.samplerAnisotropy) 
    {
        std::cout << "DeviceManager: Device does not support sampler anisotropy." << std::endl;
    }

    return indices.IsComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

bool DeviceManager::CheckDeviceExtensionSupport(VkPhysicalDevice pDev)
{
    //サポートされている拡張機能を取得
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDev, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(pDev, nullptr, &extensionCount, availableExtensions.data());

    //要求する拡張機能がサポートされているか確認
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Utils::QueueFamilyIndices DeviceManager::FindQueueFamilies(VkPhysicalDevice pDev) const
{
    Utils::QueueFamilyIndices indices;

    //キューファミリーのプロパティを取得
    uint32_t qFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDev, &qFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> qFamilies(qFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDev, &qFamilyCount, qFamilies.data());

    //キューファミリーを調査
    int i = 0;
    for (const auto& qFamily : qFamilies)
    {
        //グラフィックスキューをサポートしているか
        if (qFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        //プレゼントキューをサポートしているか
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(pDev, i, surface, &presentSupport);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("DeviceManager: Failed to query physical device surface support for queue family index " + std::to_string(i));
        }

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.IsComplete())
        {
            break;
        }

        i++;
    }

    if (!indices.IsComplete()) 
    {
        std::cout << "DeviceManager: Queue families are incomplete for the selected device." << std::endl;
    }

    return indices;
}

void DeviceManager::PickPhysicalDevice()
{
    //利用可能な物理デバイスを取得
    uint32_t pDevCount = 0;
    vkEnumeratePhysicalDevices(instance, &pDevCount, nullptr);

    std::cout << "DeviceManager: Searching for GPUs with Vulkan support" << std::endl;
    if (pDevCount == 0)
    {
        throw std::runtime_error("DeviceManager: Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(pDevCount);
    vkEnumeratePhysicalDevices(instance, &pDevCount, devices.data());

    //利用可能な物理デバイスから最適なものを１つ選択
    for (const auto& dev : devices)
    {
        VkPhysicalDeviceProperties devProperties;
        vkGetPhysicalDeviceProperties(dev, &devProperties);
        std::cout << "DeviceManager: Device Name: " << devProperties.deviceName << std::endl;

        if (IsDeviceSuitable(dev))
        {
            physicalDevice = dev;
            std::cout << "DeviceManager: Selected physical device: " << devProperties.deviceName << std::endl;
            break;
        }
        std::cout << "DeviceManager: Device " << devProperties.deviceName << " is not suitable" << std::endl;
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("DeviceManager: Failed to find a suitable GPU!");
    }

    rrRef.RegisterResource(RegistryKeys::PhysicalDevice, physicalDevice);
}

void DeviceManager::CreateLogicalDevice()
{
    if (physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("DeviceManager: Cannot create logical device without a valid physical device");
    }

    //作成するキューの設定
    std::vector<VkDeviceQueueCreateInfo> queueInfo;
    std::set<uint32_t> uniqueQFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float qPriority = 1.0f;
    for (uint32_t qFamily : uniqueQFamilies)
    {
        VkDeviceQueueCreateInfo qInfo{};
        qInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qInfo.queueCount       = 1;
        qInfo.queueFamilyIndex = qFamily;
        qInfo.pQueuePriorities = &qPriority;
        queueInfo.push_back(qInfo);
    }

    //論理デバイスに必要な機能を指定
    VkPhysicalDeviceFeatures devFeatures{};
    devFeatures.samplerAnisotropy = VK_TRUE;

    //作成する論理デバイスの設定
    VkDeviceCreateInfo devInfo{};
    devInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pQueueCreateInfos       = queueInfo.data();
    devInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueInfo.size());
    devInfo.pEnabledFeatures        = &devFeatures;
    devInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    devInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //論理デバイス作成、ResoruceRegistryに登録
    VkResult result = vkCreateDevice(physicalDevice, &devInfo, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("DeviceManager: Failed to create logical device!");
    }
    rrRef.RegisterResource(RegistryKeys::Device, device);

    //デバイスからキューを取得し、ResoruceRegistryに登録
    VkQueue graphicsQ, presentQ;
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQ);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQ);
    rrRef.RegisterResource(RegistryKeys::GraphicsQueue, graphicsQ);
    rrRef.RegisterResource(RegistryKeys::PresentQueue, presentQ);

    std::cout << "DeviceManager: Logical device created successfully." << std::endl;
}