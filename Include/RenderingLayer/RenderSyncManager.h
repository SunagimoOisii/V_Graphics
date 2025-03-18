#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>

/// <summary>
/// レンダリングプロセスにおけるセマフォとフェンスを管理するクラス<para></para>
/// スワップチェインのフレームごとに同期オブジェクトを生成・管理し、
/// レンダリングの準備と完了を制御する
/// </summary>
class RenderSyncManager
{
public:
    RenderSyncManager(VkDevice d, int maxFramesInFlight);
    ~RenderSyncManager();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    VkSemaphore GetImageAvailableSemaphore(int currentFrame) const
    {
        return imageAvailableSemaphores[currentFrame];
    }
    VkSemaphore GetRenderFinishedSemaphore(int currentFrame) const
    {
        return renderFinishedSemaphores[currentFrame];
    }
    VkFence     GetInFlightFence(int currentFrame) const
    {
        return inFlightFences[currentFrame];
    }

//***********************************************************
// 関数↓
//***********************************************************

    void CreateSyncObjects();

    void WaitForInFlightFence(int currentFrame);
    void ResetInFlightFence(int currentFrame);
    void SetMaxFramesInFlight(int newMaxFrames);

private:
//***********************************************************
// 変数↓
//***********************************************************

    int maxFramesInFlight;
    
    VkDevice device;

    //スワップチェインから画像取得しレンダリング準備完了を知らせる
    std::vector<VkSemaphore> imageAvailableSemaphores;
    //レンダリングが終了しプレゼンテーション可能状態を知らせる
    std::vector<VkSemaphore> renderFinishedSemaphores;

    //一度に1フレームのみレンダリングさせるようにする
    std::vector<VkFence> inFlightFences;
};