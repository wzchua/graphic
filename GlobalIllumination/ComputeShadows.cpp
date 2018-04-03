#include "ComputeShadows.h"
#include "LogStruct.h"
#include "CounterBlock.h"

void ComputeShadows::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    compShaderString << cameraUniformBlockShaderCodeString(GlobalShaderComponents::CAMERA_UBO_BINDING) << GlobalShaderComponents::getLightShadowMatrixUBOCode()<< Scene::getLightUBOCode(GlobalShaderComponents::LIGHT_UBO_BINDING);
    compShaderString << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    shader.generateShader(compShaderString, "./Shaders/DirectLightingWithShadows.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();

    glGenBuffers(1, &camBlkBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, camBlkBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ComputeShadows::run(Scene & inputScene, GBuffer & gBuffer)
{
    shader.use();
    auto & res = inputScene.cam.getResolution();
    GLuint output = gBuffer.getAdditionalBuffers(GBuffer::ALBEDO_SHADOWS);

    camBlk.camPosition = glm::vec4(inputScene.cam.getPosition(), 1.0f);
    camBlk.camForward = glm::vec4(inputScene.cam.getForward(), 1.0f);
    camBlk.camUp = glm::vec4(inputScene.cam.getUp(), 1.0f);
    camBlk.height = res.y;
    camBlk.width = res.x;
    glNamedBufferSubData(camBlkBufferId, 0, sizeof(CameraBlock), &camBlk);
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::CAMERA_UBO_BINDING, camBlkBufferId); //scene cam matrices
    gBuffer.bindGBuffersAsTexture(0, 1, 2, 3);
    glBindImageTexture(0, output, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glm::ivec2 setOfWorkGroups = glm::ivec2(ceil(res.x * 1.0f / mWorkGroupSize.x), ceil(res.y * 1.0f / mWorkGroupSize.y));

    
    int numOfPointLight = inputScene.getTotalPointLights();
    for (int i = 0; i < numOfPointLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getPointLightBufferId(i));
        for (int j = 0; j < 6; j++) {
            RSM& rsm = inputScene.getPointLightRSM(i, j);
            glBindTextureUnit(4, rsm.getDepthMap());
            glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_SHADOW_BINDING, rsm.getShadowMatrixBufferId());
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
            glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, 1);
        }
    }

    int numOfDirectionalLight = inputScene.getTotalDirectionalLights();
    for (int i = 0; i < numOfDirectionalLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getDirectionalLightBufferId(i));
        RSM& rsm = inputScene.getDirectionalLightRSM(i);
        glBindTextureUnit(4, rsm.getDepthMap());
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_SHADOW_BINDING, rsm.getShadowMatrixBufferId());
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, 1);
    }
}
