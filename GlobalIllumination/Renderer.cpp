#include "Renderer.h"

void Renderer::setToPhongShader()
{
    type = RenderType::PHONG;
    currentShaderProgram = phongShader.getProgramId();

    glViewport(0, 0, viewWidth, viewHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    scene.updateLightToGPU(currentShaderProgram);
}

void Renderer::phongRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 modelViewMat = scene.cam.getViewMatrix() * scene.getSceneModelMat();
    glm::mat4 modelViewProjMat = scene.cam.getProjMatrix() * modelViewMat;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glUniformMatrix3fv(glGetUniformLocation(currentShaderProgram, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));

    scene.render(currentShaderProgram);
}

Renderer::Renderer(GLFWwindow * window, unsigned int viewHeight, unsigned int viewWidth) : viewHeight(viewHeight), viewWidth(viewWidth)
{
    this->window = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKeyStatic);
    glfwSetCursorPosCallback(window, onCursorPositionStatic);
    setToPhongShader();
    
    int w = 1, h = 1;
    unsigned char image[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &nullTextureId);
    glBindTexture(GL_TEXTURE_2D, nullTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);
    glBindTexture(GL_TEXTURE_2D, 0);
    scene.nullTextureId = nullTextureId;
}

Renderer::~Renderer()
{
    glDeleteTextures(1, &nullTextureId);
}

void Renderer::startRenderLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        switch (type)
        {
        case Renderer::PHONG:
            phongRender();
            break;
        default:
            break;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::loadScene(std::string filename)
{
    scene.LoadObjScene(filename);
}

void Renderer::onCursorPosition(GLFWwindow* window, double xpos, double ypos) {
    scene.cam.viewMove(xpos, ypos);
    glfwSetCursorPos(window, 0.0, 0.0);
}

void Renderer::onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
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
            scene.cam.move(camMove);
        }
    }
}