#include <stdexcept>
#include <iostream>

#include "SystemIntegrationLayer/VulkanInitializeManager.h"
#include "ResourceRegistry.h"
#include "Utilities.h"

//***************************************************************************************************************************
// �g���֐�(�f�o�b�O���b�Z���W���[)
//***************************************************************************************************************************
//�g���֐��͊g���@�\�̈ꕔ�������[�_�[���C�u�����Ɋ܂܂�Ă��Ȃ��B
//����Ɋ֐��|�C���^(vkGetInstanceProcAddr�֐����g�p)��p���Ď��s���ɓ��I�Ƀ��[�h���s���B

    //�f�o�b�O���b�Z���W���[��Vulkan�C���X�^���X�Ƃ��̃��C���[�ɌŗL�Ȃ̂ő������ŃC���X�^���X���w��
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
// public��
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
// private��
//************************************************************************************************************************

void VulkanInitializeManager::CreateInstance() 
{
    //�o���f�[�V�������C���[�̃T�|�[�g�m�F
    if (enableValidationLayers && 
        !CheckValidationLayerSupport()) 
    {
        throw std::runtime_error("InitializeManager: Validation layers requested, but not available!");
    }

    //�A�v���P�[�V��������ݒ�
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 250);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    //�쐬����C���X�^���X�̐ݒ�
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    //�K�v�Ȋg���@�\���擾���ݒ�
    auto extensions = FetchRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    //�o���f�[�V�������C���[�ƃf�o�b�O���b�Z���W���[�̐ݒ�
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

    //�C���X�^���X�쐬�AResourceRegistry�ɓo�^
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    rrRef.RegisterResource(RegistryKeys::VkInstance, instance);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

bool VulkanInitializeManager::CheckValidationLayerSupport()
{
    //�o���f�[�V�������C���[�̃��X�g���擾
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    //�v�����郌�C���[�����p�\�����m�F
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
    //GLFW���C�u�������K�v�Ƃ���g���@�\�̎擾
    if (!glfwInit())
    {
        throw std::runtime_error("InitializeManager: Failed to initialize GLFW");
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    //���C�g���[�V���O�ɕK�v�Ȋg���@�\�̎擾
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    //�o���f�[�V�������C���[�L�����Ƀf�o�b�O�o�̓R�[���o�b�N�̊g���@�\�ǉ�
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

    //�f�o�b�O���b�Z���W���[�̍쐬
    VkResult result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("InitializeManager: Failed to set up debug messenger!");
    }
}

void VulkanInitializeManager::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    //�f�o�b�O���b�Z�[�W�̏d��x�ƃ^�C�v��ݒ�
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