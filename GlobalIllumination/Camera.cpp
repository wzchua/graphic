#include "Camera.h"

Camera::Camera(glm::vec3 position) : position(position)
{

}

Camera::~Camera()
{
}

void Camera::moveTo(glm::vec3 position)
{
    isViewMatrixUpdated = false;
    this->position = position;
}

void Camera::move(glm::vec3 translation)
{
    isViewMatrixUpdated = false;
    position = position + translation.z * forward + translation.x * right;
}

void Camera::viewMove(int mouseXDelta, int mouseYDelta)
{
    isViewMatrixUpdated = false;
    double sensitivityX = -0.02;
    double sensitivityY = 0.01;
    yRot += mouseXDelta * sensitivityX;
    xRot += mouseYDelta * sensitivityY;
    xRot = (xRot > 0) ? glm::min(90.0f, xRot) : glm::max(-90.0f, xRot);

    glm::quat qPitch = glm::angleAxis(xRot, glm::vec3(1.0, 0.0, 0.0));
    glm::quat qYaw = glm::angleAxis(yRot, glm::vec3(0.0, 1.0, 0.0));
    glm::quat qRotate = glm::normalize(qYaw * qPitch);

    forward = glm::normalize(qRotate * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    right = glm::normalize(qRotate * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    up = glm::normalize(qRotate * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

}

glm::mat4 Camera::getViewMatrix() const
{
    if (isViewMatrixUpdated) {
        return viewMatrix;
    }

    glm::vec3 at = position + forward;
    viewMatrix = glm::lookAt(position,
        at,
        up);
    isViewMatrixUpdated = true;
    return viewMatrix;
}

glm::mat4 Camera::getProjMatrix() const
{
    if (isProjMatrixUpdated) {
        return projMatrix;
    }

    projMatrix = glm::perspective(glm::radians(fovDegree), aspect, nearPlane, farPlane);
    isProjMatrixUpdated = true;
    return projMatrix;
}
