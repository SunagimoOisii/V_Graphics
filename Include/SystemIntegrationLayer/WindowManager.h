#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class WindowManager
{
public:
    WindowManager(VkInstance instance,
        uint32_t width, uint32_t height, const char* title,
        class ResourceRegistry& rrRef);
    ~WindowManager();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    void SetRenderer(class Renderer* r) { resizeRenderer = r; }

//***********************************************************
// 関数↓
//***********************************************************

    void PollEvents() const;

private:
//***********************************************************
// 変数↓
//***********************************************************

    GLFWwindow*  window;
    VkSurfaceKHR surface;

    class Renderer* resizeRenderer;
    class ResourceRegistry& rrRef;

//***********************************************************
// 関数↓
//***********************************************************

    void InitWindow(uint32_t width, uint32_t height, const char* title);

    void CreateSurface(VkInstance instance);

    static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
};