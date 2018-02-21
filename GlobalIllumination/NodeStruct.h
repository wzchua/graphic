#pragma once
#include <glad\glad.h>

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