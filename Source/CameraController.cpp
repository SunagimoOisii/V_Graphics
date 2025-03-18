#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "KeyboardManager.h"
#include "MouseManager.h"
#include "Camera.h"
#include "CameraController.h"

CameraController::CameraController(Camera& cam, float moveS)
    : camera(cam)
    , moveSpeed(moveS) 
{
}

CameraController::~CameraController()
{
}

void CameraController::Update(const KeyboardManager& input, const MouseManager& mouse, float deltaTime)
{
    float speed = moveSpeed * deltaTime;
    glm::vec3 pos      = camera.GetPosition();
    glm::vec3 forward  = glm::normalize(camera.GetTarget() - pos);
    glm::vec3 right    = glm::normalize(glm::cross(forward, up));

    //std::cout << "Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
    //std::cout << "Target: " << camera.GetTarget().x << ", " << camera.GetTarget().y << ", " << camera.GetTarget().z << std::endl;

    //WASD�ŃJ�����ړ�
    if (input.IsDown(GLFW_KEY_W)) pos += speed * forward;
    if (input.IsDown(GLFW_KEY_S)) pos -= speed * forward;
    if (input.IsDown(GLFW_KEY_A)) pos -= speed * right;
    if (input.IsDown(GLFW_KEY_D)) pos += speed * right;

    //Shift,Space��Y���ړ�
    if (input.IsDown(GLFW_KEY_SPACE))      pos += speed * up;
    if (input.IsDown(GLFW_KEY_LEFT_SHIFT)) pos -= speed * up;

    //�}�E�X�h���b�O�ɂ�鎋�_��]
    if (mouse.IsDown(GLFW_MOUSE_BUTTON_LEFT) ||
        mouse.IsDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        double deltaX, deltaY;
        mouse.GetCursorDelta(deltaX, deltaY);

        const float sensitivity = 0.1f;
        yaw   += static_cast<float>(deltaX) * sensitivity;
        pitch -= static_cast<float>(deltaY) * sensitivity; //�㉺���]���邽�ߕ������t��

        //�s�b�`�p�̐���
        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        //std::cout << "Yaw: " << yaw << ", Pitch: " << pitch << std::endl;

        //�J�����̐V�����������v�Z
        glm::vec3 dir{};
        dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        dir.y = sin(glm::radians(pitch));
        dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = glm::normalize(dir);
    }

    camera.SetPosition(pos);
    camera.SetTarget(pos + forward);
}