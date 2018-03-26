#include "RenderPhong.h"



void RenderPhong::initialize()
{
    phongShader.generateShader("./Shaders/Phong.vert", ShaderProgram::VERTEX);
    phongShader.generateShader("./Shaders/Phong.frag", ShaderProgram::FRAGMENT);
    phongShader.linkCompileValidate();

}

void RenderPhong::setup(Scene & scene, GLuint viewWidth, GLuint viewHeight)
{
    int currentShaderProgram = phongShader.use();

    glViewport(0, 0, viewWidth, viewHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, scene.getMatrixBuffer());
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, scene.getLightBuffer());
}

void RenderPhong::run(Scene & scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (num == 0) {
        glViewport(0, 0, 800, 600);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, scene.getMatrixBuffer());
        scene.updateMatrixBuffer();
    }
    else {
        glViewport(0, 0, 1024, 1024);
        switch (num) {
        case 1:
            scene.updateLightMatrixBuffer(0, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
            break;
        case 2:
            scene.updateLightMatrixBuffer(0, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
            break;
        case 3:
            scene.updateLightMatrixBuffer(0, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
            break;
        case 4:
            scene.updateLightMatrixBuffer(0, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
            break;
        case 5:
            scene.updateLightMatrixBuffer(0, glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
            break;
        case 6:
            scene.updateLightMatrixBuffer(0, glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
            break;
        }
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, scene.getLightMatrixBuffer()); // light as camera
    }
    scene.render(phongShader.getProgramId());
}

void RenderPhong::onNumberPressed(int num)
{
    this->num = num;
}

RenderPhong::RenderPhong()
{
}


RenderPhong::~RenderPhong()
{
}
