#pragma once

#include <unordered_map>

/// <summary>
/// �}�E�X�̃N���b�N��,�J�[�\���ړ��ʂ��v�Z,�񋟂���N���X
/// </summary>
class MouseManager
{
public:
    MouseManager(GLFWwindow& winRef);
    ~MouseManager();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
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
// �֐���
//***********************************************************

    void Update();

    void ApplyCursorLock(bool lock);

    bool IsPressed(int key)  const;
    bool IsDown(int key)     const;
    bool IsReleased(int key) const;
    bool IsIdle(int key)     const;

private:
//***********************************************************
// �ϐ���
//***********************************************************
 
    GLFWwindow& windowRef;

    double previousX, previousY; //�O�t���[�����W
    double currentX, currentY;   //���t���[�����W

    bool cursorLocked;

    //�O,���t���[���ł́A����L�[�̉�����Ԃ�\������
    struct KeyState
    {
        bool previous;
        bool current;
    };
    std::unordered_map<int, KeyState> keyStateTraceMap;
};