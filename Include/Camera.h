#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// <summary>
/// �J�����Ƃ��Ă̍��W,�����_,���[���h���W�n�ɂ����������̒P�ʃx�N�g���̏��ƁA
/// ���������ƂɃr���[�s����v�Z����@�\�����N���X
/// </summary>
class Camera
{
public:
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up);
    ~Camera();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    glm::vec3 GetPosition()   const { return position; };
    glm::vec3 GetTarget()     const { return target; };

    void SetPosition(const glm::vec3& pos) { position = pos; };
    void SetTarget(const glm::vec3& tar) { target = tar; };

//***********************************************************
// �֐���
//***********************************************************

    glm::mat4 CalcViewMatrix() const;

private:
//***********************************************************
// �ϐ���
//***********************************************************

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};