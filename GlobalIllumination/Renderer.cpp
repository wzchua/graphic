#include "Renderer.h"

void Renderer::setToPhongShader()
{
    type = RenderType::PHONG; 
    mModuleRenderPhong.setup(scene, viewWidth, viewHeight);
}

void Renderer::phongRender()
{
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();

    mModuleRenderPhong.run(scene);

    auto timeEnd = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - timeStart).count();
    std::cout << "time after render: " << timeEnd << "ns" << std::endl;

}

void Renderer::voxelRender() {
    voxelizer.render(scene);
}

void Renderer::setToVoxelizeShader() {
    type = RenderType::VOXELIZE;
}

Renderer::Renderer(GLFWwindow * window, unsigned int viewHeight, unsigned int viewWidth) : viewHeight(viewHeight), viewWidth(viewWidth)
{
    this->window = window;;
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKeyStatic);
    glfwSetCursorPosCallback(window, onCursorPositionStatic);

    mModuleRenderPhong.initialize();

    //setToPhongShader();
    setToVoxelizeShader();
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
            camMove.z += 5.0f;
        }
        if (key == GLFW_KEY_S) {
            camMove.z -= 5.0f;
        }
        if (key == GLFW_KEY_A) {
            camMove.x += 5.0f;
        }
        if (key == GLFW_KEY_D) {
            camMove.x -= 5.0f;
        }
        // doesn't register stimulateous presses
        if (camMove.z != 0.0f || camMove.x != 0.0f) {
            scene.cam.move(camMove);
        }
        if (key == GLFW_KEY_F1) {
            setToPhongShader();
        }
        if (key == GLFW_KEY_F2){
            setToVoxelizeShader();
        }
        if (type == Renderer::VOXELIZE) {
            if (key == GLFW_KEY_0) {
                voxelizer.onNumberPressed(0);
            }
            if (key == GLFW_KEY_1) {
                voxelizer.onNumberPressed(1);
            }
            if (key == GLFW_KEY_2) {
                voxelizer.onNumberPressed(2);
            }
            if (key == GLFW_KEY_3) {
                voxelizer.onNumberPressed(3);
            }
            if (key == GLFW_KEY_4) {
                voxelizer.onNumberPressed(4);
            }
        }
        else if (type == Renderer::PHONG) {
            if (key == GLFW_KEY_0) {
                mModuleRenderPhong.onNumberPressed(0);
            }
            if (key == GLFW_KEY_1) {
                mModuleRenderPhong.onNumberPressed(1);
            }
            if (key == GLFW_KEY_2) {
                mModuleRenderPhong.onNumberPressed(2);
            }
            if (key == GLFW_KEY_3) {
                mModuleRenderPhong.onNumberPressed(3);
            }
            if (key == GLFW_KEY_4) {
                mModuleRenderPhong.onNumberPressed(4);
            }
            if (key == GLFW_KEY_5) {
                mModuleRenderPhong.onNumberPressed(5);
            }
            if (key == GLFW_KEY_6) {
                mModuleRenderPhong.onNumberPressed(6);
            }


        }

    }
}