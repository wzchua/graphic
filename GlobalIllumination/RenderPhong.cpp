#include "RenderPhong.h"



void RenderPhong::initialize()
{
    phongShader.generateShader("./Shaders/Phong.vert", ShaderProgram::VERTEX);
    phongShader.generateShader("./Shaders/Phong.frag", ShaderProgram::FRAGMENT);
    phongShader.linkCompileValidate();
}

void RenderPhong::run(Scene & scene, GLuint viewWidth, GLuint viewHeight)
{
    int currentShaderProgram = phongShader.use();

    glViewport(0, 0, viewWidth, viewHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    scene.updateLightToGPU(currentShaderProgram);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 modelViewMat = scene.cam.getViewMatrix() * scene.getSceneModelMat();
    glm::mat4 modelViewProjMat = scene.cam.getProjMatrix() * modelViewMat;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glUniformMatrix3fv(glGetUniformLocation(currentShaderProgram, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));

    scene.render(currentShaderProgram);
}

RenderPhong::RenderPhong()
{
}


RenderPhong::~RenderPhong()
{
}
