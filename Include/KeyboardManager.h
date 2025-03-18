#include <GLFW/glfw3.h>
#include <unordered_map>

/// <summary>
/// �L�[�̏�Ԃ�ǐ�,�񋟂���N���X<para></para>
/// �Ȃ��A�ǐՂ������L�[�͎��O�ɓo�^����K�v������
/// </summary>
class KeyboardManager
{
public:
    KeyboardManager(GLFWwindow& winRef);
    ~KeyboardManager();

//***********************************************************
// �֐���
//***********************************************************

    //��Ԃ�ǐՂ������L�[��o�^����
    void RegisterKey(int key);

    void Update();

    bool IsPressed(int key)  const;
    bool IsDown(int key)     const;
    bool IsReleased(int key) const;
    bool IsIdle(int key)     const;

private:
//***********************************************************
// �ϐ���
//***********************************************************

    GLFWwindow& windowRef;

    //�O,���t���[���ł́A����L�[�̉�����Ԃ�\������
    struct KeyState
    {
        bool previous;
        bool current;
    };
    std::unordered_map<int, KeyState> keyStateTraceMap;
};