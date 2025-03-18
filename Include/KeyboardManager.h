#include <GLFW/glfw3.h>
#include <unordered_map>

/// <summary>
/// キーの状態を追跡,提供するクラス<para></para>
/// なお、追跡したいキーは事前に登録する必要がある
/// </summary>
class KeyboardManager
{
public:
    KeyboardManager(GLFWwindow& winRef);
    ~KeyboardManager();

//***********************************************************
// 関数↓
//***********************************************************

    //状態を追跡したいキーを登録する
    void RegisterKey(int key);

    void Update();

    bool IsPressed(int key)  const;
    bool IsDown(int key)     const;
    bool IsReleased(int key) const;
    bool IsIdle(int key)     const;

private:
//***********************************************************
// 変数↓
//***********************************************************

    GLFWwindow& windowRef;

    //前,現フレームでの、あるキーの押下状態を表現する
    struct KeyState
    {
        bool previous;
        bool current;
    };
    std::unordered_map<int, KeyState> keyStateTraceMap;
};