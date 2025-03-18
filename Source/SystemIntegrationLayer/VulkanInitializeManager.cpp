#include <stdexcept>
#include <iostream>

#include "SystemIntegrationLayer/VulkanInitializeManager.h"
#include "ResourceRegistry.h"
#include "Utilities.h"

//***************************************************************************************************************************
// 拡張関数(デバッグメッセンジャー)
//***************************************************************************************************************************
//拡張関数は拡張機能の一部だがローダーライブラリに含まれていない。
//代わりに関数ポインタ(vkGetInstanceProcAddr関数を使用)を用いて実行時に動的にロードを行う。

    //デバッグメッセンジャーはVulkanインスタンスとそのレイヤーに固有なので第一引数でインスタンスを指定
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

VulkanInitializeManager::VulkanInitializeManager(bool enableValidationLayers, class ResourceRegistry& rrRef)
    : debugMessenger(VK_NULL_HANDLE)
    , enableValidationLayers(enableValidationLayers)
    , instance(VK_NULL_HANDLE)
    , rrRef(rrRef)
{
}

VulkanInitializeManager::~VulkanInitializeManager()
{
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanInitializeManager::Initialize()
{
    CreateInstance();
    SetupDebugMessenger();
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void VulkanInitializeManager::CreateInstance() 
{
    //バリデーションレイヤーのサポート確認
    if (enableValidationLayers && 
        !CheckValidationLayerSupport()) 
    {
        throw std::runtime_error("InitializeManager: Validation layers requested, but not available!");
    }

    //アプリケーション情報を設定
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 250);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    //作成するインスタンスの設定
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    //必要な拡張機能を取得し設定
    auto extensions = FetchRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    //バリデーションレイヤーとデバッグメッセンジャーの設定
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    //インスタンス作成、ResourceRegistryに登録
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    rrRef.RegisterResource(RegistryKeys::VkInstance, instance);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

bool VulkanInitializeManager::CheckValidationLayerSupport()
{
    //バリデーションレイヤーのリストを取得
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    //要求するレイヤーが利用可能かを確認
    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanInitializeManager::FetchRequiredExtensions() const
{
    //GLFWライブラリが必要とする拡張機能の取得
    if (!glfwInit())
    {
        throw std::runtime_error("InitializeManager: Failed to initialize GLFW");
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    //レイトレーシングに必要な拡張機能の取得
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    //バリデーションレイヤー有効時にデバッグ出力コールバックの拡張機能追加
    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanInitializeManager::SetupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);

    //デバッグメッセンジャーの作成
    VkResult result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("InitializeManager: Failed to set up debug messenger!");
    }
}

void VulkanInitializeManager::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    //デバッグメッセージの重大度とタイプを設定
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &DebugCallback;
}