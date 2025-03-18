#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

/// <summary>
/// Vulkan�C���X�^���X�ƃf�o�b�O���b�Z���W���[���Ǘ�����
/// </summary>
class VulkanInitializeManager
{
public:
    VulkanInitializeManager(bool enableValidationLayers, class ResourceRegistry& rrRef);
    ~VulkanInitializeManager();

//***********************************************************
// �֐���
//***********************************************************

    void Initialize();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableValidationLayers;
    const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    class ResourceRegistry& rrRef;

//***********************************************************
// �֐���
//***********************************************************

    void CreateInstance();

    bool CheckValidationLayerSupport();

    /// <summary>
    /// �쐬����f�o�b�O���b�Z���W���[�̐ݒ���s��
    /// </summary>
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void SetupDebugMessenger();

    std::vector<const char*> FetchRequiredExtensions() const;
};