#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
class Shader
{
private:
    unsigned int shaderProgram;
public:
    unsigned int getProgramId();
    Shader(std::string vertexShaderFilename, std::string fragShaderFilename);
    ~Shader();
    GLuint generateVertexShader(std::string filename);
    GLuint generateFragmentShader(std::string filename);
};

