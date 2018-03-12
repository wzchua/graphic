#include "ShaderProgram.h"

std::string getShaderLog(GLuint shaderId) {
    // Compile failed, get log
    int length = 0;
    std::string logString;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        char * c_log = new char[length];
        int written = 0;
        glGetShaderInfoLog(shaderId, length, &written, c_log);
        logString = c_log;
        delete[] c_log;
    }
    return logString;
}
std::string getProgramLog(GLuint programId) {
    // Link failed, get log
    int length = 0;
    std::string logString;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        char * c_log = new char[length];
        int written = 0;
        glGetProgramInfoLog(programId, length, &written, c_log);
        logString = c_log;
        delete[] c_log;
    }
    return logString;
}

void ShaderProgram::linkCompileValidate()
{
    for (auto & shader : shaders) {
        glAttachShader(shaderProgram, shader.getShaderId());
    }
    glLinkProgram(shaderProgram);
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        std::cout << "Program Link Failed.\n Log: " << getProgramLog(shaderProgram) << "\n";
        throw new std::exception();
    }
    for (auto & shader : shaders) {
        glDetachShader(shaderProgram, shader.getShaderId());
    }

    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        std::cout << "Program validate Failed.\n Log: " << getProgramLog(shaderProgram) << "\n";
        throw new std::exception();
    }
}

unsigned int ShaderProgram::use()
{
    glUseProgram(shaderProgram);
    return shaderProgram;
}

unsigned int ShaderProgram::getProgramId()
{
    return shaderProgram;
}

ShaderProgram::ShaderProgram()
{
    shaderProgram = glCreateProgram();
    shaders.reserve(5);
}

ShaderProgram::~ShaderProgram()
{
    if (shaderProgram == 0) return;

    for (auto & shader : shaders) {
        shader.destroy();
    }

    GLint numShaders = 0;
    glGetProgramiv(shaderProgram, GL_ATTACHED_SHADERS, &numShaders);

    // Get the shader names
    GLuint * shaderNames = new GLuint[numShaders];
    glGetAttachedShaders(shaderProgram, numShaders, NULL, shaderNames);

    // Delete the shaders
    for (int i = 0; i < numShaders; i++)
        glDeleteShader(shaderNames[i]);

    // Delete the program
    glDeleteProgram(shaderProgram);

    delete[] shaderNames;
}

void ShaderProgram::generateShader(std::string filename, ShaderType type)
{
    shaders.push_back(Shader());
    shaders.back().initializeWithFileName(filename, type);
}
void ShaderProgram::generateShader(std::string & headerString, std::string filename, ShaderType type)
{
    shaders.push_back(Shader());
    shaders.back().initializeWithFileName(headerString, filename, type);
}

void ShaderProgram::generateShader(std::stringstream & headerString, std::string filename, ShaderProgram::ShaderType type)
{
    shaders.push_back(Shader());
    shaders.back().initializeWithFileName(headerString, filename, type);
}

void ShaderProgram::Shader::initializeWithFileName(std::string filename, ShaderType type)
{
    shaderId = glCreateShader(type);
    this->type = type;
    std::ifstream in(filename, std::ifstream::in);
    std::cout << filename << "\n";
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();
    const auto vString = buffer.str();
    const char * source = vString.c_str();
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << filename << " Compile Failed.\n Log: " << getShaderLog(shaderId) << "\n";
        throw new std::exception();
    }
    buffer.clear();
}

void ShaderProgram::Shader::initializeWithFileName(std::string & headerString, std::string filename, ShaderType type)
{
    shaderId = glCreateShader(type);
    this->type = type;
    std::ifstream in(filename, std::ifstream::in);
    std::cout << filename << "\n";
    std::stringstream buffer;
    buffer << headerString;
    buffer << in.rdbuf();
    in.close();
    const auto vString = buffer.str();
    const char * source = vString.c_str();
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << filename << " Compile Failed.\n Log: " << getShaderLog(shaderId) << "\n";
        throw new std::exception();
    }
    buffer.clear();
}

void ShaderProgram::Shader::initializeWithFileName(std::stringstream & buffer, std::string filename, ShaderType type)
{
    shaderId = glCreateShader(type);
    this->type = type;
    std::ifstream in(filename, std::ifstream::in);
    std::cout << filename << "\n";
    buffer << in.rdbuf();
    in.close();
    const auto vString = buffer.str();
    const char * source = vString.c_str();
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << filename << " Compile Failed.\n Log: " << getShaderLog(shaderId) << "\n";
        throw new std::exception();
    }
}

ShaderProgram::Shader::~Shader()
{

}

GLuint ShaderProgram::Shader::getShaderId()
{
    return shaderId;
}

void ShaderProgram::Shader::destroy() {
    if (shaderId == 0) {
        return;
    }
    glDeleteShader(shaderId);
    shaderId = 0;
}