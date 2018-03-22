#pragma once
#include <glad/glad.h>
#include <string>

struct CounterBlock {
    GLuint fragmentCounter;
    GLuint nodeCounter;
    GLuint brickCounter;
    GLuint leafCounter;
    GLuint logCounter;
    GLuint noOfFragments;
};
static std::string counterBlockBufferShaderCodeString(GLuint level) {
    std::string s = R"(layout(binding = )" + std::to_string(level);
    s = s + R"() coherent buffer CounterBlock{
    uint fragmentCounter;
    uint nodeCounter;
    uint brickCounter;
    uint leafCounter;
    uint logCounter;
    uint noOfFragments;
};
)";
    return s;
}