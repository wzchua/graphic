#include "RenderLightTextures.h"


void RenderLightTextures::initialize()
{
    if (hasInitialized) {
        return;
    }
    shader.generateShader("./Shaders/RSM.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/RSM.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();

    glCreateRenderbuffers(1, &rboId);
    glNamedRenderbufferStorage(rboId, GL_DEPTH_COMPONENT, rsmRes.x, rsmRes.y);

    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, rsmRes.x);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, rsmRes.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderLightTextures::run(Scene & inputScene, glm::ivec2 res, std::vector<GLuint>& depthMap, std::vector<GLuint>& positionMap, std::vector<GLuint>& normalMap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, res.x, res.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    for (int i = 0; i < depthMap.size(); i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getLightMatrixBuffer()); // light as camera
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[0], 0);

        glBindImageTexture(0, positionMap[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA);
        glBindImageTexture(1, normalMap[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA);

        inputScene.updateLightMatrixBuffer(0, forward[i], up[i]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputScene.render(shader.getProgramId());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderLightTextures::RenderLightTextures()
{
}


RenderLightTextures::~RenderLightTextures()
{
}
