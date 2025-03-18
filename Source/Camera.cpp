#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : position(position)
    , target(target)
    , up(up) 
{
}

Camera::~Camera()
{
}

glm::mat4 Camera::CalcViewMatrix() const
{
    return glm::lookAt(position, target, up);
}