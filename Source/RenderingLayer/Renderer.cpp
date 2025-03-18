#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS //glm::rotate等の関数がラジアンを引数として使うことを確認するために必要
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //投影変換にまつわる関数を使用

#define STB_IMAGE_IMPLEMENTATION //関数本体をインクルードするために必要
#include <stb_image.h>

#include <chrono>

#include "RenderingLayer/Renderer.h"

#include "Camera.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

Renderer::Renderer(ResourceRegistry& rrRef, VkDevice d, Camera& cam, int maxFramesInFlight,
    VkBuffer vertexB, VkBuffer indexB, const std::vector<uint16_t> indices,
    RenderSyncManager* rsm,
    std::function<void(uint32_t, const Utils::MVPUBO&)> uploadMVPUniformBuffersF,
    std::function<const std::vector<VkFramebuffer>()> getFrameBuffersF,
    std::function<void()> recreateRenderingFlowF)
    : device(d)
    , indices(indices)
    , vertexB(vertexB)
    , indexB(indexB)
    , uploadMVPUniformBuffersFunc(uploadMVPUniformBuffersF)
    , windowResized(false)
    , maxFramesInFlight(maxFramesInFlight)
    , cameraRef(cam)
    , rrRef(rrRef)
    , getFrameBuffersFunc(getFrameBuffersF)
    , rsm(rsm)
    , recreateRenderingFlowFunc(recreateRenderingFlowF)
{
}

Renderer::~Renderer()
{
}

void Renderer::Render(bool& loopFlag, float deltaTime)
{
//ウィンドウの状態に応じて遷移
    auto window = rrRef.FindResource<GLFWwindow*>(RegistryKeys::Window);
    if (window &&
        !glfwWindowShouldClose(window)) //開
    {
        glfwPollEvents();
        DrawFrame();
    }
    else //閉
    {
        loopFlag = false;
    }
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void Renderer::UpdateUniformBuffer(uint32_t currentImg)
{
    //レンダリング開始からの時間を秒単位で計算
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime      = std::chrono::high_resolution_clock::now();
    float time            = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    //モデル,ビュー,プロジェクションの情報を持つ構造体を用いて回転
    Utils::MVPUBO mvpUBO{};
    auto swapChainExtent = rrRef.FindResource<VkExtent2D>(RegistryKeys::SwapChainExtent);
    //mvpUBO.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //回転
    mvpUBO.model = glm::mat4(1.0f);
    mvpUBO.view  = cameraRef.CalcViewMatrix();
    mvpUBO.proj  = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 15.0f);
    mvpUBO.proj[1][1] *= -1; //Y軸を反転してVulkanのNDCに対応

    //ライトの動きを設定
    float radius = 1.0f;
    float speed = 0.5f;

    //ライトの方向
    mvpUBO.direction = glm::vec3(
        0.0f,
        1.0f, // Y軸は固定
        radius * sin(time * speed)
    );

    //ライトの色（時間に応じて変化）
    mvpUBO.color = glm::vec3(
        (sin(time * 0.5f) + 1.0f) * 0.5f,  // R: 0.0〜1.0
        (cos(time * 0.7f) + 1.0f) * 0.5f,  // G: 0.0〜1.0
        (sin(time * 0.9f) + 1.0f) * 0.5f   // B: 0.0〜1.0
    );

    //ライトの強度
    mvpUBO.intensity = 1.0f;

    uploadMVPUniformBuffersFunc(currentImg, mvpUBO);
    //drawResourceLayer->UploadMVPUniformBuffers(currentImg, mvpUBO);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer cBuf, uint32_t imgIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = 0; //コマンドバッファの使用法
    beginInfo.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer(cBuf, &beginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Renderer: failed to begin recording CommandBuffer!");
    }

//レンダーパスの詳細入力,開始
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = rrRef.FindResource<VkRenderPass>(RegistryKeys::RenderPass);
    //renderPassInfo.framebuffer       = renderingLayer->GetFrameBuffers()[imgIndex];
    renderPassInfo.framebuffer = getFrameBuffersFunc()[imgIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = rrRef.FindResource<VkExtent2D>(RegistryKeys::SwapChainExtent);

    //クリア値(アタッチメントの順番と同じにする必要アリ)
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
        //アタッチメントでVK_ATTACHMENT_LOAD_OP_CLEARの設定がある場合に使用するクリア値
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(cBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    auto graphicsPipeline = rrRef.FindResource<VkPipeline>(RegistryKeys::GraphicsPipeline);
    vkCmdBindPipeline(cBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

//動的ステートにする機能の設定
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    auto swapChainExtent = rrRef.FindResource<VkExtent2D>(RegistryKeys::SwapChainExtent);
    viewport.width    = static_cast<float>(swapChainExtent.width);
    viewport.height   = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cBuf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(cBuf, 0, 1, &scissor);

//コマンドバッファと頂点バッファのバインド
    VkBuffer vertexBuffers[] = { vertexB };
    VkDeviceSize offsets[]   = { 0 };
    vkCmdBindVertexBuffers(cBuf, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cBuf, indexB, 0, VK_INDEX_TYPE_UINT16);

    auto descriptorSets = rrRef.FindResource<std::vector<VkDescriptorSet>>(RegistryKeys::DescriptorSets);
    auto pipelineLayout = rrRef.FindResource<VkPipelineLayout>(RegistryKeys::GraphicsPipelineLayout);
    vkCmdBindDescriptorSets(cBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(cBuf, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(cBuf);

    result = vkEndCommandBuffer(cBuf);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Renderer: failed to record command buffer!");
    }
}

void Renderer::DrawFrame()
{
    rsm->WaitForInFlightFence(currentFrame);

//スワップチェインから画像取得
    uint32_t imageIndex;
    VkSemaphore imgAvailableSemaphore = rsm->GetImageAvailableSemaphore(currentFrame);
    auto swapChain = rrRef.FindResource<VkSwapchainKHR>(RegistryKeys::SwapChain);
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imgAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    //ウィンドウサイズの変更等でスワップチェインとサーフェスの互換性がなくなった場合,スワップチェイン再作成
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateRenderingFlowFunc();
        return;
    }
    else if (result != VK_SUCCESS &&
             result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("falied to Acquire SwapChainImage!");
    }

    UpdateUniformBuffer(currentFrame);

    //1フレーム画像を取得した時点でフェンス発動
    rsm->ResetInFlightFence(currentFrame);

//コマンドバッファの前回までの内容を消去してから入力受け付け開始
    auto commandBuffers = rrRef.FindResource<std::vector<VkCommandBuffer>>(RegistryKeys::CommandBuffers);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

//コマンドバッファ送信
    VkSubmitInfo submitInfo{};
    VkSemaphore semaphore             = rsm->GetRenderFinishedSemaphore(currentFrame);
    VkSemaphore waitSemaphores[]      = { imgAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[]    = { semaphore };

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //セマフォ
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages; //パイプライン上で待機するステージ
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores; //コマンドバッファ実行終了後どのセマフォにシグナルを送るか
    //コマンドバッファの情報
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffers[currentFrame];

    auto graphicsQ = rrRef.FindResource<VkQueue>(RegistryKeys::GraphicsQueue);
    VkFence fence  = rsm->GetInFlightFence(currentFrame);
    result = vkQueueSubmit(graphicsQ, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Renderer: failed to submit draw command buffer!");
    }
    else
    {
        std::cout << "Renderer: Submitted DrawCommandBuffer" << std::endl;
    }

//プレゼンテーション
    VkPresentInfoKHR presentInfo{};
    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    //セマフォ
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    //スワップチェイン
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains    = swapChains;
    presentInfo.pImageIndices  = &imageIndex;
    presentInfo.pResults       = nullptr;

    auto presentQ = rrRef.FindResource<VkQueue>(RegistryKeys::PresentQueue);
    result = vkQueuePresentKHR(presentQ, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || 
        result == VK_SUBOPTIMAL_KHR ||
        windowResized)
    {
        windowResized = false;
        recreateRenderingFlowFunc();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Renderer: failed to present SwapChainImage!");
    }

    currentFrame = (currentFrame + 1) % maxFramesInFlight;
}