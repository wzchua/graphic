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
        void initializeWithFileName(std::string filename, ShaderType shaderType);
        void initializeWithFileName(std::string & headerString, std::string filename, ShaderType shaderType);
        void initializeWithFileName(std::stringstream & buffer, std::string filename, ShaderType shaderType);
        Shader() : shaderId(0) {}
        void destroy();
    };
    void generateShader(std::string filename, ShaderProgram::ShaderType type);
    void generateShader(std::string & headerString, std::string filename, ShaderProgram::ShaderType type);
    void generateShader(std::stringstream & headerString, std::string filename, ShaderProgram::ShaderType type);
    void linkCompileValidate();
    unsigned int use();
    unsigned int getProgramId();
    ShaderProgram();
    ~ShaderProgram();
private:
    unsigned int shaderProgram = 0;
    std::vector<Shader> shaders;
};

