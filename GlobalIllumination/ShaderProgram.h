#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>
class ShaderProgram
{
public:
    enum ShaderType {
        VERTEX = GL_VERTEX_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        COMPUTE = GL_COMPUTE_SHADER
    };
    class Shader {
    private:
        GLuint shaderId;
        ShaderType type;
    public:
        ~Shader();
        static std::string shaderRootPath;
        GLuint getShaderId();    
        void initialize(std::string filename, ShaderType shaderType, const std::vector<std::string>* newDefines);
        Shader() : shaderId(0) {}
        void destroy();
    };
    void generateShader(std::string filename, ShaderProgram::ShaderType type, const std::vector<std::string>* newDefines = nullptr);
    void linkCompileValidate();
    unsigned int use();
    unsigned int getProgramId();
    ShaderProgram();
    ~ShaderProgram();
private:
    unsigned int shaderProgram = 0;
    std::vector<Shader> shaders;
};

