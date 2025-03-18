#include "KeyboardManager.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

KeyboardManager::KeyboardManager(GLFWwindow& winRef)
    : windowRef(winRef)
{
}

KeyboardManager::~KeyboardManager()
{
}

void KeyboardManager::RegisterKey(int key)
{
    keyStateTraceMap[key] = { false, false };
}

void KeyboardManager::Update()
{
    //状態追跡が有効なキーの状態を更新
    for (auto& [key, state] : keyStateTraceMap)
    {
        state.previous = state.current;
        state.current  = glfwGetKey(&windowRef, key) == GLFW_PRESS;
    }
}

bool KeyboardManager::IsPressed(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return !it->second.previous && it->second.current;
    }
    return false;
}

bool KeyboardManager::IsDown(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return it->second.previous && it->second.current;
    }
    return false;
}

bool KeyboardManager::IsReleased(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return it->second.previous && !it->second.current;
    }
    return false;
}

bool KeyboardManager::IsIdle(int key) const
{
    auto it = keyStateTraceMap.find(key);
    if (it != keyStateTraceMap.end())
    {
        return !it->second.previous && !it->second.current;
    }
    return false;
}