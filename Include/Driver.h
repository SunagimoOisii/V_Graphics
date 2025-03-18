#pragma once

#include <chrono>
#include <memory>

#include "ResourceRegistry.h"

#include "SceneLayer/SceneLayer.h"
#include "RenderingLayer/RenderingLayer.h"
#include "DrawResourceLayer/DrawResourceLayer.h"
#include "SystemIntegrationLayer/SystemIntegrationLayer.h"
#include "HardwareAbstractionLayer/HardwareAbstractionLayer.h"

class Driver
{
public:
    Driver();
    ~Driver();

    bool loopFlag = true;

    void Loop();

private:
    const uint32_t INIT_WIDTH  = 800;
    const uint32_t INIT_HEIGHT = 600;
    const char* WINDOW_TITLE   = "Vulkan Test";
    const int MAX_FRAMES_IN_FLIGHT = 2; //同時に処理するフレーム数
    static constexpr float FrameRate    = 120.0f;
    static constexpr float MaxDeltaTime = 0.05f;
    float deltaTime = 0;
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;

    const std::vector<Vertex> vertices =
    {
        //板ポリ1枚目(法線:+Z)
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        // 板ポリ2枚目(法線:-Z方向)
        {{-0.5f, -0.5f, -1.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},

        // 床(法線:+Y)
        {{-5.0f,  1.0f, -5.0f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 5.0f,  1.0f, -5.0f}, {0.5f, 0.5f, 0.5f}, {5.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 5.0f,  1.0f,  5.0f}, {0.5f, 0.5f, 0.5f}, {5.0f, 5.0f}, {0.0f, 1.0f, 0.0f}},
        {{-5.0f,  1.0f,  5.0f}, {0.5f, 0.5f, 0.5f}, {0.0f, 5.0f}, {0.0f, 1.0f, 0.0f}},
    };

    const std::vector<uint16_t> indices =
    {
        //板ポリ1枚目
        0, 1, 2, 2, 3, 0,
        //板ポリ2枚目
        4, 5, 6, 6, 7, 4,
        //床
        8, 9, 10, 10, 11, 8
    };

    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;

    std::unique_ptr<class KeyboardManager>  keyboardManager;
    std::unique_ptr<class MouseManager>     mouseManager;

    std::unique_ptr<class Camera>           camera;
    std::unique_ptr<class CameraController> cameraController;

    std::unique_ptr<ResourceRegistry> rr;

    std::unique_ptr<SceneLayer>               sceneLayer;
    std::unique_ptr<RenderingLayer>           renderingLayer;
    std::unique_ptr<DrawResourceLayer>        drawResourceLayer;
    std::unique_ptr<HardwareAbstractionLayer> hardwareAbstractionLayer;
    std::unique_ptr<SystemIntegrationLayer>   systemIntegrationLayer;

    bool Tick(float& deltaTime);
};