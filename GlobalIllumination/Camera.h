#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
private:
    glm::vec3 position;
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0);
    float xRot = 0.0f;
    float yRot = 0.0f;
    float fovDegree = 60.0f;
    float aspect = 4.0f / 3.0f;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;

    mutable bool isViewMatrixUpdated = false;
    mutable glm::mat4 viewMatrix;
    mutable bool isProjMatrixUpdated = false;
    mutable glm::mat4 projMatrix;
public:
    Camera(glm::vec3 position);
    ~Camera();
    void moveTo(glm::vec3 position);
    void move(glm::vec3 translation);
    void viewMove(int mouseXDelta, int mouseYDelta);
    glm::vec3 getPosition();
    glm::vec3 getForward();
    glm::vec3 getUp();

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjMatrix() const;
};