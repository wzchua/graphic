#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene.h"
#include "ShaderProgram.h"
#include "Voxelizer.h"
#include "RenderPhong.h"

class Renderer
{
private:
    enum RenderType { PHONG, VOXELIZE };
    RenderType type;
    GLFWwindow * window;
    Scene scene;
    void phongRender();
    void voxelRender();
    unsigned int currentShaderProgram;
    unsigned int viewHeight;
    unsigned int viewWidth;
    GLuint nullTextureId;
    Voxelizer voxelizer;
    RenderPhong mModuleRenderPhong;

    void onCursorPosition(GLFWwindow* window, double xpos, double ypos);
    void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    inline static void onKeyStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Renderer * ptr = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
        ptr->onKey(window, key, scancode, action, mods);
    }
    inline static void onCursorPositionStatic(GLFWwindow* window, double xpos, double ypos) {
        Renderer * ptr = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
        ptr->onCursorPosition(window, xpos, ypos);
    }
public:
    Renderer(GLFWwindow * window, unsigned int viewHeight, unsigned int viewWidth);
    ~Renderer();
    void startRenderLoop();
    void loadScene(std::string filename);
    void setToPhongShader();
    void setToVoxelizeShader();
};

