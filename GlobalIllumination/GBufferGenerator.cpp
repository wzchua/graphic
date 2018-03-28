#include "GBufferGenerator.h"



void GBufferGenerator::initialize()
{
    if (hasInitialized) {
        return;
    }

    shader.generateShader("./Shaders/DeferredPhongDiffuse.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/DeferredPhongDiffuse.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
}

void GBufferGenerator::run(Scene & inputScene, GBuffer & gBuffer, GLuint voxelMatrixUniformBuffer)
{
    auto res = inputScene.cam.getResolution();
    gBuffer.initialize(res.x, res.y);
    inputScene.updateMatrixBuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.getFboId());
    glViewport(0, 0, res.x, res.y);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getMatrixBuffer());
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, voxelMatrixUniformBuffer);
    inputScene.render(shader.getProgramId());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBufferGenerator::GBufferGenerator()
{
}


GBufferGenerator::~GBufferGenerator()
{
}
