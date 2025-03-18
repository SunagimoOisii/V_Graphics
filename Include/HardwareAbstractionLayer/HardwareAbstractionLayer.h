#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "DeviceManager.h"
#include "CommandResourceManager.h"

/// <summary>
/// �n�[�h�E�F�A���\�[�X�𓝊��Ǘ����郌�C���[���b�v�N���X<para></para>
/// Vulkan�f�o�C�X�A�������A�R�}���h���\�[�X������������ђ񋟂��������S��<para></para>
/// SystemIntegrationLayer�ŏ��������ꂽVulkan�C���X�^���X��T�[�t�F�X�����ƂɁA<para></para>
/// �f�o�C�X�֘A���\�[�X�𒊏ۉ�����
/// </summary>
class HardwareAbstractionLayer
{
public:
    HardwareAbstractionLayer(class ResourceRegistry& rrRef);
    ~HardwareAbstractionLayer();

private:
//***********************************************************
// �ϐ���
//***********************************************************

    std::unique_ptr<DeviceManager>           deviceManager;
    std::unique_ptr<CommandResourceManager>  commandResourceManager;
};