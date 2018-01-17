#include "Renderer.h"

void Renderer::setToPhongShader()
{
    type = RenderType::PHONG;
    currentShaderProgram = phongShader.use();

    glViewport(0, 0, viewWidth, viewHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    scene.updateLightToGPU(currentShaderProgram);
}

void Renderer::phongRender()
{
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 modelViewMat = scene.cam.getViewMatrix() * scene.getSceneModelMat();
    glm::mat4 modelViewProjMat = scene.cam.getProjMatrix() * modelViewMat;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glUniformMatrix3fv(glGetUniformLocation(currentShaderProgram, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));

    scene.render(currentShaderProgram);
    auto timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    std::cout << "time after render: " << timeEnd << "ms" << std::endl;

}

void Renderer::voxelRender() {
    voxelizer.voxelizeFragmentList(scene);
}

void Renderer::setToVoxelizeShader() {
    type = RenderType::VOXELIZE;
    scene.cam = voxelizer.camVoxel;
}

Renderer::Renderer(GLFWwindow * window, unsigned int viewHeight, unsigned int viewWidth) : viewHeight(viewHeight), viewWidth(viewWidth)
{
    this->window = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKeyStatic);
    glfwSetCursorPosCallback(window, onCursorPositionStatic);
    phongShader.generateShader("./Shaders/Phong.vert", ShaderProgram::VERTEX);
    phongShader.generateShader("./Shaders/Phong.frag", ShaderProgram::FRAGMENT);
    phongShader.linkCompileValidate();

    setToPhongShader();
    //setToVoxelizeShader();
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
        case Renderer::VOXELIZE:
            voxelRender();
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
    voxelizer.initializeWithScene(scene.getSceneMinCoords(), scene.getSceneMaxCoords());
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

        if (key == GLFW_KEY_1) {
            voxelizer.projectionAxis = 0;
        }
        if (key == GLFW_KEY_2) {
            voxelizer.projectionAxis = 1;
        }
        if (key == GLFW_KEY_3) {
            voxelizer.projectionAxis = 2;
        }
    }
}