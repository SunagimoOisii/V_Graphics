#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <array>
#include <string>
#include <vector>
#include <optional>

#include "Vertex.h"
#include "Utilities.h"

#include "ResourceRegistry.h"

#include "RenderingLayer/RenderingLayer.h"
#include "RenderingLayer/FrameBufferManager.h"

//各データごとのアライメント要件
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout

class Renderer
{
public:
    Renderer(class ResourceRegistry& rrRef, VkDevice d, class Camera& cam, int maxFramesInFlight,
        VkBuffer vertexB, VkBuffer indexB, const std::vector<uint16_t> indices,
        RenderSyncManager* rsm,
        std::function<void(uint32_t, const Utils::MVPUBO&)> uploadMVPUniformBuffersF,
        std::function<const std::vector<VkFramebuffer> ()> getFrameBuffersF,
        std::function<void()> recreateRenderingFlowF);
    ~Renderer();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    void SetFrameBufferResized(bool resized) { windowResized = resized; }

//***********************************************************
// 関数↓
//***********************************************************

    void Render(bool& loopFlag, float deltaTime);

private:
//***********************************************************
// 変数↓
//***********************************************************

    bool windowResized;

    int maxFramesInFlight;
    uint32_t currentFrame = 0;

    VkDevice device;

    VkBuffer vertexB;
    VkBuffer indexB;

    const std::vector<uint16_t> indices;

    std::function<void(uint32_t, const Utils::MVPUBO&)> uploadMVPUniformBuffersFunc;
    std::function<const std::vector<VkFramebuffer>()>   getFrameBuffersFunc;
    std::function<void()>                               recreateRenderingFlowFunc;

    class ResourceRegistry& rrRef;
    class Camera&           cameraRef;

    class RenderSyncManager* rsm;

//***********************************************************
// 関数↓
//***********************************************************

    void UpdateUniformBuffer(uint32_t currentImg);

    void RecordCommandBuffer(VkCommandBuffer cBuf, uint32_t imgIndex);

    void DrawFrame();
};