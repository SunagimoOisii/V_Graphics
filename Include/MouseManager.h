#pragma once

#include <unordered_map>

/// <summary>
/// マウスのクリック状況,カーソル移動量を計算,提供するクラス
/// </summary>
class MouseManager
{
public:
    MouseManager(GLFWwindow& winRef);
    ~MouseManager();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    void GetCursorPosition(double& x, double& y) const
    {
        x = currentX;
        y = currentY;
    }
    void GetCursorDelta(double& dx, double& dy) const
    {
        dx = currentX - previousX;
        dy = currentY - previousY;
    }

//***********************************************************
// 関数↓
//***********************************************************

    void Update();

    void ApplyCursorLock(bool lock);

    bool IsPressed(int key)  const;
    bool IsDown(int key)     const;
    bool IsReleased(int key) const;
    bool IsIdle(int key)     const;

private:
//***********************************************************
// 変数↓
//***********************************************************
 
    GLFWwindow& windowRef;

    double previousX, previousY; //前フレーム座標
    double currentX, currentY;   //現フレーム座標

    bool cursorLocked;

    //前,現フレームでの、あるキーの押下状態を表現する
    struct KeyState
    {
        bool previous;
        bool current;
    };
    std::unordered_map<int, KeyState> keyStateTraceMap;
};