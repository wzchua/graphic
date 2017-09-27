#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
class PhongShader
{
private:
    std::string vertShaderFilename = "./Shaders/Phong.vert";
    std::string fragShaderFilename = "./Shaders/Phong.frag";
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
public:
    unsigned int getProgramId();
    PhongShader();
    ~PhongShader();
};

