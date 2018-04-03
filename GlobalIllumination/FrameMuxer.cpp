#include "FrameMuxer.h"

void FrameMuxer::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    shader.generateShader(compShaderString, "./Shaders/AddTextures.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();
}

void FrameMuxer::run(GBuffer& gBuffer)
{
    GLuint albedoWithShadows = gBuffer.getAdditionalBuffers(0);
    GLuint indirect = gBuffer.getAdditionalBuffers(1);
    GLuint finalImage = gBuffer.getFinalTextureId();
    auto size = gBuffer.getSize();
    shader.use();

    glBindTextureUnit(0, albedoWithShadows);
    glBindTextureUnit(1, indirect);
    glBindImageTexture(0, finalImage, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glDispatchCompute(size.x / mWorkGroupSize.x, size.y / mWorkGroupSize.y, 1);
}
