#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>

const std::vector<std::string> textureNames = { "Test" };

//ResourceRegistry�N���X�Ŋe�탊�\�[�X�ɑΉ������L�[���`���Ă���
namespace RegistryKeys
{
//***********************************************************
// �`��ݒ�n
//***********************************************************

    constexpr const char* MaxFramesInFlight = "MaxFramesInFlight";

//***********************************************************
// SystemIntegrationLayer�n
//***********************************************************

    constexpr const char* Window     = "Window";
    constexpr const char* Surface    = "Surface";
    constexpr const char* VkInstance = "VkInstance";

//***********************************************************
// HardwareAbstractionLayer�n
//***********************************************************

    constexpr const char* Device             = "Device";
    constexpr const char* PhysicalDevice     = "PhysicalDevice";
    constexpr const char* GraphicsQueue      = "GraphicsQueue";
    constexpr const char* PresentQueue       = "PresentQueue";
    constexpr const char* QueueFamilyIndices = "QueueFamilyIndices";

    constexpr const char* CommandPool    = "CommandPool";
    constexpr const char* CommandBuffers = "CommandBuffers";

//***********************************************************
// DrawResourceLayer�n
//***********************************************************

    constexpr const char* MVPUBs = "MVPUniformBuffers";

//***********************************************************
// RenderingLayer�n
//***********************************************************

    constexpr const char* SwapChain       = "SwapChain";
    constexpr const char* SwapChainExtent = "SwapChainExtent";

    constexpr const char* RenderPass = "RenderPass";

    constexpr const char* GraphicsPipeline       = "GraphicsPipeline";
    constexpr const char* GraphicsPipelineLayout = "GraphicsPipelineLayout";

    constexpr const char* DescriptorSetLayout = "DescriptorSetLayout";
    constexpr const char* DescriptorSets      = "DescriptorSets";
}

namespace Utils
{
//***********************************************************
// �\���́�
//***********************************************************

    /// <summary>
    /// �T�[�t�F�X�ڍ׏���ێ�����\����
    /// </summary>
    struct SurfaceCapabilities
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /// <summary>
    /// �L���[�t�@�~���[�̃C���f�b�N�X��ێ�����\����
    /// </summary>
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

//****************************
// UniformBufferObject��
//****************************
    //�e�V�F�[�_�X�e�[�W���f�B�X�N���v�^�z���ɎQ�Ƃ���

    struct MVPUBO
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;

        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
        float intensity;
    };

    struct GlobalLightUBO
    {
        alignas(16) glm::vec3 color;
        alignas(4)  float intensity;
    };

    struct DirectionalLightUBO
    {
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
        float intensity;
    };

//***********************************************************
// �֐���
//***********************************************************

    /// <summary>
    /// �����ɍ��v���郁�����^�C�v�̃C���f�b�N�X���擾����
    /// </summary>
    /// <param name="typeFilter">�������^�C�v�̃r�b�g�}�X�N</param>
    /// <param name="properties">�v�����郁�����v���p�e�B</param>
    /// <returns>���������������^�C�v�̃C���f�b�N�X</returns>
    uint32_t FindMemoryType(VkPhysicalDevice pd, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /// <summary>
    /// �����f�o�C�X�ŃT�|�[�g����Ă���t�H�[�}�b�g����������
    /// </summary>
    /// <param name="candidates">�T�|�[�g���Ă���t�H�[�}�b�g�̃��X�g</param>
    /// <param name="tiling">�摜�̃^�C�����O���@</param>
    /// <param name="features">�K�v�ȃt�H�[�}�b�g�@�\�t���O</param>
    /// <returns>�T�|�[�g����Ă���t�H�[�}�b�g</returns>
    VkFormat FindSupportedFormat(VkPhysicalDevice pd, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat FindDepthFormat(VkPhysicalDevice pd);

    Utils::SurfaceCapabilities QuerySurfaceSupport(VkPhysicalDevice pd, VkSurfaceKHR sur);

    /// <summary>
    /// �ꎞ�I�ȃR�}���h�o�b�t�@���J�n
    /// </summary>
    /// <returns>�L�^�\�ȏ�Ԃ̃R�}���h�o�b�t�@</returns>
    VkCommandBuffer BeginTransientCommands(VkDevice d, VkCommandPool cp);

    /// <summary>
    /// �ꎞ�I�ȃR�}���h�o�b�t�@�̋L�^���I�����A�L���[�ɒ�o�����チ�����J��
    /// </summary>
    void EndTransientCommands(VkDevice d, VkQueue graphicsQ, VkCommandPool cp, VkCommandBuffer cb);

    std::vector<char> ReadFile(const std::string& fileName);
}