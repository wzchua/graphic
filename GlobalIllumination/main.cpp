#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

#include "ObjLoader.h"
#include "Shape.h"
#include "Camera.h"

int setupPhongShader();
std::string getShaderLog(GLuint shaderId);
std::string getProgramLog(GLuint programId);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const double X_MID = SCR_WIDTH / 2;
const double Y_MID = SCR_HEIGHT / 2;
std::string filepath;

const GLfloat lightPosition[] = { 10.0f, 5.0f, 10.0f, 0.0f };   // Directional light. Given in eye space.
const GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
const GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

Camera cam(glm::vec3(5.0f));

static void WaitForEnterKeyBeforeExit(void)
{
    fflush(stdin);
    getchar();
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        exit(-1);
    }
    filepath = std::string(argv[1]);
    std::cout << filepath << std::endl;
    atexit(WaitForEnterKeyBeforeExit); // std::atexit() is declared in cstdlib
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyboardCallback); 
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetCursorPos(window, 0.0, 0.0);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    if (!GLAD_GL_VERSION_4_5) {
        std::cout << "OpenGL not version 4.5" << std::endl;
        exit(-1);
    }
    int programId = setupPhongShader();

    glm::mat4 modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.2, 0.2, 0.2));

    glUniform4fv(glGetUniformLocation(programId, "LightPosition"), 1, lightPosition);
    glUniform4fv(glGetUniformLocation(programId, "LightAmbient"), 1, lightAmbient);
    glUniform4fv(glGetUniformLocation(programId, "LightDiffuse"), 1, lightDiffuse);
    glUniform4fv(glGetUniformLocation(programId, "LightSpecular"), 1, lightSpecular);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    std::map<std::string, GLuint> textureMap;
    std::vector<Shape> list;
    ObjLoader::loadObj(filepath, list, textureMap);


    for(int i = 0; i < list.size(); i++) {
        list[i].generateGPUBuffers();
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelViewMat = cam.getViewMatrix() * modelMat;
        glm::mat4 modelViewProjMat = cam.getProjMatrix() * modelViewMat;
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

        glUniformMatrix4fv(glGetUniformLocation(programId, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
        glUniformMatrix4fv(glGetUniformLocation(programId, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
        glUniformMatrix3fv(glGetUniformLocation(programId, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));

        for (int i = 0; i < list.size(); i++) {
            list[i].render(programId, textureMap);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

int setupPhongShader() {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream in ("./Shaders/Phong.vert", std::ifstream::in);
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

    std::ifstream inFrag = std::ifstream("./Shaders/Phong.frag", std::ifstream::in);
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
    int shaderProgram = glCreateProgram();
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

    return shaderProgram;
}

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

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    cam.viewMove(xpos, ypos);
    glfwSetCursorPos(window, 0.0, 0.0);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Close window
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        glm::vec3 camMove;
        if (key == GLFW_KEY_W) {
            camMove.z += 1.0f;
        }
        if (key == GLFW_KEY_S) {
            camMove.z -= 1.0f;
        }
        if (key == GLFW_KEY_A) {
            camMove.x += 1.0f;
        }
        if (key == GLFW_KEY_D) {
            camMove.x -= 1.0f;
        }
        // doesn't register stimulateous presses
        if (camMove.z != 0.0f || camMove.x != 0.0f) {
            cam.move(camMove);
        }
    }
}