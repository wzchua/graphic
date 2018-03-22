#pragma once
#include <glm\glm.hpp>
#include <glad/glad.h>
#include <string>

struct CameraBlock {
    glm::vec4 camPosition;
    glm::vec4 camForward;
    glm::vec4 camUp;
    int height;
    int width;
};
static std::string cameraUniformBlockShaderCodeString(GLuint level) {
    std::string s = R"(layout(binding = )" + std::to_string(level);
    s = s + R"() uniform CameraBlock {
    vec4 camPosition;
    vec4 camForward;
    vec4 camUp;
    int height;
    int width;
};
)";
    return s;
}