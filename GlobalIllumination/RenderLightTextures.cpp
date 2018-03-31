#include "RenderLightTextures.h"


void RenderLightTextures::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getVertTripleInputs() << GlobalShaderComponents::getVertToFragTripleOutput();
    vertShaderString << GlobalShaderComponents::getLightCamMatrixUBOCode();
    shader.generateShader(vertShaderString, "./Shaders/RSM.vert", ShaderProgram::VERTEX);

    std::stringstream fragShaderString;
    fragShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getFragTripleInput();
    fragShaderString << GlobalShaderComponents::getMaterialUBOCode();
    fragShaderString << voxelizeBlockString(GlobalShaderComponents::VOXELIZATION_MATRIX_UBO_BINDING);
    shader.generateShader(fragShaderString, "./Shaders/RSM.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();

    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, rsmRes.x);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, rsmRes.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderLightTextures::run(Scene & inputScene)
{
    shader.use();
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, rsmRes.x, rsmRes.y);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDrawBuffers(2, attachments);
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_CAMERA_MATRIX_UBO_BINDING, inputScene.getLightMatrixBuffer()); // light as camera
    int numOfPointLight = inputScene.getTotalPointLights();

    for (int i = 0; i < numOfPointLight; i++) {
        for (int j = 0; j < 6; j++) {
            RSM& rsm = inputScene.getPointLightRSM(i, j);
            rsm.initialize(rsmRes);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rsm.getDepthMap(), 0);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rsm.getVoxelPositionMap(), 0);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, rsm.getNormalMap(), 0);

            inputScene.updateLightMatrixBufferForPointLight(i, forward[j], up[j]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            inputScene.render(shader.getProgramId());
            glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
            glGenerateTextureMipmap(rsm.getVoxelPositionMap());
            glGenerateTextureMipmap(rsm.getNormalMap());
            //rsm.dumpAsImage(std::to_string(i) + "_(" + std::to_string(j) + ")");
        }
    }
    int numOfDirectionalLight = inputScene.getTotalDirectionalLights();
    for (int i = 0; i < numOfDirectionalLight; i++) {
        RSM& rsm = inputScene.getDirectionalLightRSM(i);
        rsm.initialize(rsmRes);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rsm.getDepthMap(), 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rsm.getVoxelPositionMap(), 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, rsm.getNormalMap(), 0);
        
        inputScene.updateLightMatrixBufferForDirectionalLight(i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputScene.render(shader.getProgramId());
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
        glGenerateTextureMipmap(rsm.getVoxelPositionMap());
        glGenerateTextureMipmap(rsm.getNormalMap());
        //rsm.dumpAsImage(std::to_string(i));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

