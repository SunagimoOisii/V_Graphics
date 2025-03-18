#pragma once
#include <memory>
#include <vulkan/vulkan.h>

#include "VulkanInitializeManager.h"
#include "WindowManager.h"

/// <summary>
/// �v���b�g�t�H�[���ˑ������𓝍����A�K�v�ȃ��\�[�X��񋟂��郌�C���[���b�v�N���X<para></para>
/// �E�B���h�E�Ǘ���Vulkan�C���X�^���X�̐�����S�����A<para></para>
/// HardwareAbstractionLayer�ɕK�v��VkInstance��VkSurfaceKHR��񋟂���
/// </summary>
class SystemIntegrationLayer
{
public:
    SystemIntegrationLayer(class ResourceRegistry& rrRef,
        bool enableValidationLayers,
        uint32_t width, uint32_t height, const char* windowTitle);
    ~SystemIntegrationLayer();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    std::unique_ptr<VulkanInitializeManager> vulkanInitializeManager;
    std::unique_ptr<WindowManager>           windowManager;
};