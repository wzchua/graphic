#pragma once
#include <glad\glad.h>
#include <string>

struct NodeStruct {
    GLuint parentPtr;
    GLuint childPtr;
    GLuint childBit;
    GLuint modelBrickPtr;
    GLuint lightBit;
    GLuint lightBrickPtr;
    GLuint xPositive;
    GLuint xNegative;
    GLuint yPositive;
    GLuint yNegative;
    GLuint zPositive;
    GLuint zNegative;
};
static std::string nodeStructShaderCodeString(GLuint level) {
    std::string s = R"(struct NodeStruct {
    uint parentPtr;
    uint childPtr;
    uint childBit;
    uint modelBrickPtr;
    uint lightBit;
    uint lightBrickPtr;
    uint xPositive;
    uint xNegative;
    uint yPositive;
    uint yNegative;
    uint zPositive;
    uint zNegative;
};
layout(binding = )" + level; 
    s = s + R"() coherent buffer NodeBlock{
    NodeStruct node[];
};
)";
    return s;
}