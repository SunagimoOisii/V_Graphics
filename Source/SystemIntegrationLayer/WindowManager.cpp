#include <stdexcept>
#include <iostream>

#include "SystemIntegrationLayer/WindowManager.h"
#include "RenderingLayer/Renderer.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

WindowManager::WindowManager(VkInstance instance,
    uint32_t width, uint32_t height, const char* title,
    ResourceRegistry& rrRef)
    : rrRef(rrRef)
{
    InitWindow(width, height, title);
    CreateSurface(instance);
}

WindowManager::~WindowManager()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowManager::PollEvents() const
{
    glfwPollEvents();
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

void WindowManager::InitWindow(uint32_t width, uint32_t height, const char* title)
{
    //Vulkanを使用するためOpenGLは無効化
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //windowを作成、ResourceRegistryに登録
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    rrRef.RegisterResource(RegistryKeys::Window, window);
    if (!window)
    {
        throw std::runtime_error("WindowManager: Failed to create GLFW window");
    }

    //リサイズコールバックの設定
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FrameBufferResizeCallback);
}

void WindowManager::CreateSurface(VkInstance instance)
{
    //surfaceを作成、ResourceRegistryに登録
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    rrRef.RegisterResource(RegistryKeys::Surface, surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("WindowManager: Failed to create window surface");
    }

    std::cout << "WindowManager: Window Surface created successfully" << std::endl;
}

void WindowManager::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //ウィンドウに紐づけられた自身のインスタンスを取得
    auto wm = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (wm && 
        wm->resizeRenderer)
    {
        //Rendererにリサイズを通知
        wm->resizeRenderer->SetFrameBufferResized(true);
    }
}