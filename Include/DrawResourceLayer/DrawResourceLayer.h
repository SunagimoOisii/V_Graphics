#pragma once
#include <memory>

#include "MemoryPool.h"

#include "BufferResourceManager.h"
#include "ImageResourceManager.h"
#include "TextureManager.h"

/// <summary>
/// �`��ɕK�v�ȃ��\�[�X�𓝊��Ǘ����郌�C���[���b�v�N���X<para></para>
/// ���_,�C���f�b�N�X�o�b�t�@�A�e�N�X�`���A�C���[�W�Ȃǂ̃��\�[�X��������ъǗ���S���A<para></para>
/// �`�揈���ɕK�v�Ȋe���񋟂��������S��
/// </summary>
class DrawResourceLayer
{
public:
    DrawResourceLayer(class ResourceRegistry& rrRef);
    ~DrawResourceLayer();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    VkImageView GetTextureImageView(const std::string& textureName);
    VkSampler   GetTextureSampler(const std::string& textureName);

//***********************************************************
// �֐���
//***********************************************************

//****************************
// �o�b�t�@(VkBuffer)�n��
//****************************

    VkBuffer CreateVertexBuffer(const std::vector<Vertex>& vertices);
    VkBuffer CreateIndexBuffer(const std::vector<uint16_t>& indices);
    void     CreateMVPUniformBuffers(size_t maxFramesInFlight);
    void     UploadMVPUniformBuffers(uint32_t currentImage, const Utils::MVPUBO& ubo);

//****************************
// �o�b�t�@(VkImage)�n��
//****************************

    std::vector<VkImageView> CreateSwapChainImageViews(const std::vector<VkImage>& scImages, VkFormat format);

    std::pair<VkImage, VkImageView> CreateColorAttachmentImage(VkExtent2D extent, VkFormat format);
    std::pair<VkImage, VkImageView> CreateDepthAttachmentImage(VkExtent2D extent);

//****************************
// �e�N�X�`���n��
//****************************

    void LoadTexture(const std::string& textureName, const std::string& filePath);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    std::unique_ptr<MemoryPool> memoryPool;

    std::unique_ptr<BufferResourceManager> bufferResourceManager;
    std::unique_ptr<ImageResourceManager>  imageResourceManager;
    std::unique_ptr<TextureManager>        textureManager;
};