#include <GLFW/glfw3.h>

#include "MouseManager.h"

//************************************************************************************************************************
// public��
//************************************************************************************************************************

MouseManager::MouseManager(GLFWwindow& winRef)
    : currentX(0.0)
    , currentY(0.0)
    , previousX(0.0)
    , previousY(0.0)
    , cursorLocked(false)
    , windowRef(winRef)
{
    //��ԒǐՂ��s���L�[�̓o�^
    KeyState initS = { false,false };
    keyStateTraceMap[GLFW_MOUSE_BUTTON_LEFT]  = initS;
    keyStateTraceMap[GLFW_MOUSE_BUTTON_RIGHT] = initS;
}

MouseManager::~MouseManager() 
{
}

void MouseManager::Update()
{
    if (cursorLocked)
    {
        return;
    }

    //�O,���J�[�\���ʒu���擾
    previousX = currentX;
    previousY = currentY;
    glfwGetCursorPos(&windowRef, &currentX, &currentY);

    //��ԒǐՂ��L���ȃL�[�̏�Ԃ��X�V
    for (auto& [key, state] : keyStateTraceMap)
    {
        state.previous = state.current;
        state.current = (glfwGetMouseButton(&windowRef, key) == GLFW_PRESS);
    }
}

void MouseManager::ApplyCursorLock(bool lock)
{
    cursorLocked = lock;
    int flag = lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, flag);
}

bool MouseManager::IsPressed(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return !it->second.previous && it->second.current;
    }
    return false;
}

bool MouseManager::IsDown(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return it->second.previous && it->second.current;
    }
    return false;
}

bool MouseManager::IsReleased(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return it->second.previous && !it->second.current;
    }
    return false;
}

bool MouseManager::IsIdle(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return !it->second.previous && !it->second.current;
    }
    return false;
}