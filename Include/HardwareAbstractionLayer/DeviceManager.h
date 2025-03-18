#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

#include "Utilities.h"

/// <summary>
/// �����E�_�n�f�o�C�X�A�L���[�̊Ǘ����s��<para></para>
/// �Ȃ��f�o�C�X�ŃT�|�[�g����Ă��郁�����^�C�v��t�H�[�}�b�g�̌����A
/// �T�[�t�F�X�̋@�\�T�|�[�g�N�G���͐ӔC�O
/// </summary>
class DeviceManager
{
public:
    DeviceManager(class ResourceRegistry& rrRef,
        VkInstance ins, VkSurfaceKHR sur);
    ~DeviceManager();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physicalDevice;
    VkDevice         device;

    Utils::QueueFamilyIndices indices;

    const std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
    };

    class ResourceRegistry& rrRef;

//***********************************************************
// �֐���
//***********************************************************

    bool IsDeviceSuitable(VkPhysicalDevice pDev);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice pDev);

    /// <summary>
    /// �����f�o�C�X�̃L���[�t�@�~���[����������
    /// </summary>
    /// <returns>���������L���[�t�@�~���[�̃C���f�b�N�X</returns>
    Utils::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pDev) const;

    void PickPhysicalDevice();

    void CreateLogicalDevice();
};