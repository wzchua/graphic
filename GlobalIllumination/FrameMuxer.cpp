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

    const GLenum attachments[1]{ GL_COLOR_ATTACHMENT0 };
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glDrawBuffers(1, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameMuxer::run(GBuffer& gBuffer)
{
    GLuint albedoWithShadows = gBuffer.getAdditionalBuffers(0);
    GLuint indirect = gBuffer.getAdditionalBuffers(1);
    GLuint finalImage = gBuffer.getAdditionalBuffers(2);
    auto size = gBuffer.getSize();
    shader.use();

    glBindTextureUnit(0, albedoWithShadows);
    glBindTextureUnit(1, indirect);
    glBindImageTexture(0, finalImage, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glDispatchCompute(size.x / mWorkGroupSize.x, size.y / mWorkGroupSize.y, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, finalImage, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glBlitNamedFramebuffer(fboId, 0, 0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
