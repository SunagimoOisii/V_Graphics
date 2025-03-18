#pragma once

/// <summary>
/// Camera�N���X�̏��,InputManager�N���X����̓��͏󋵂���
/// �J�����̈ړ���]���s���N���X
/// </summary>
class CameraController
{
public:
    CameraController(class Camera& cam, float moveSpeed);
    ~CameraController();

    void Update(const class KeyboardManager& input, const class MouseManager& mouse, float deltaTime);

private:
    class Camera& camera;

    float moveSpeed;

    glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);
    float yaw   = 90.0f;
    float pitch = 0.0f;
};