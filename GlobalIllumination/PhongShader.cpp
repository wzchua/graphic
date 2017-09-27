#include "PhongShader.h"

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

PhongShader::PhongShader()
{
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream in(vertShaderFilename, std::ifstream::in);
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();
    const auto vString = buffer.str();
    const char * source = vString.c_str();
    glShaderSource(vertexShader, 1, &source, NULL);
    glCompileShader(vertexShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << "Vertex Shader Compile Failed.\n Log: " << getShaderLog(vertexShader) << "\n";
        exit(-1);
    }
    buffer.clear();
    buffer.str(std::string());

    std::ifstream inFrag = std::ifstream(fragShaderFilename, std::ifstream::in);
    buffer << inFrag.rdbuf();
    in.close();

    const auto fString = buffer.str();
    const char * fragSource = fString.c_str();
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << "Fragment Shader Compile Failed.\n Log: " << getShaderLog(fragmentShader) << "\n";
        exit(-1);
    }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        std::cout << "Program Link Failed.\n Log: " << getProgramLog(shaderProgram) << "\n";
        exit(-1);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        std::cout << "Program validate Failed.\n Log: " << getProgramLog(shaderProgram) << "\n";
        exit(-1);
    }
    glUseProgram(shaderProgram);
}


PhongShader::~PhongShader()
{
    if (shaderProgram == 0) return;

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

unsigned int PhongShader::getProgramId() 
{
    return shaderProgram;
}