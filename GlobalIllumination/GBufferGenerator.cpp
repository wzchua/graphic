#include "GBufferGenerator.h"



void GBufferGenerator::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToFragTripleOutput();
    vertShaderString << GlobalCom::getCamMatrixUBOCode();
    shader.generateShader(vertShaderString, "./Shaders/DeferredPhongDiffuse.vert", ShaderProgram::VERTEX);

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getMaterialUBOCode();
    fragShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX_UBO_BINDING);
    shader.generateShader(fragShaderString, "./Shaders/DeferredPhongDiffuse.frag", ShaderProgram::FRAGMENT);

    shader.linkCompileValidate();
}

void GBufferGenerator::run(Scene & inputScene, GBuffer & gBuffer)
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
    inputScene.render(shader.getProgramId());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
