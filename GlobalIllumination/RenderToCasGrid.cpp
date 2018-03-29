#include "RenderToCasGrid.h"

void RenderToCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToGeomTripleOutput();
    vertShaderString << voxelizeBlockString(GlobalCom::CAS_VOXELIZATION_MATRIX_UBO_BINDING) << GlobalCom::getGlobalVariablesUBOCode();
    vertShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);

    shader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GlobalCom::getHeader() << GlobalCom::getGeomTripleInput() << GlobalCom::getGeomToFragTripleOutput();
    shader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::string colorTex = "layout(binding = " + std::to_string(GlobalCom::COLOR_IMAGE_BINDING) + ", r32ui) uniform coherent volatile uimage3D colorBrick;\n";
    std::string normalTex = "layout(binding = " + std::to_string(GlobalCom::NORMAL_IMAGE_BINDING) + ", r32ui) uniform coherent volatile uimage3D normalBrick;\n";

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getMaterialUBOCode() << GlobalCom::getGlobalVariablesUBOCode();
    fragShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    fragShaderString << colorTex << normalTex;
    shader.generateShader(fragShaderString, "./Shaders/VoxelizeCasGrid.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
    //OpenGLQueryObject::getShaderBuffersInfo(shader.getProgramId());
}
// Suggestion: Generate the cascaded grid based on cam world position map
void RenderToCasGrid::run(Scene & inputScene, glm::mat4 & worldToVoxelMat, CascadedGrid & casGrid)
{
    auto & cam = inputScene.cam;
    auto refPos = glm::vec3(worldToVoxelMat * glm::vec4(cam.getPosition(), 1.0f)) + 8.0f * cam.getForward();
    casGrid.setRefCamPosition(glm::vec4(refPos, 1.0f), worldToVoxelMat);

    GLuint currentShaderProgram = shader.use();

    GLuint dim = casGrid.getClipDimensions();
    GLuint numOfGrid = casGrid.getCascadedLevels();
    auto& voxelMatrixBlockIds = casGrid.getVoxelMatrixBlockIds();
    auto& textureColors = casGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto& textureNormals = casGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);


    glViewport(0, 0, dim, dim);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    for (int i = 0; i < numOfGrid; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalCom::CAS_VOXELIZATION_MATRIX_UBO_BINDING, voxelMatrixBlockIds[i]);
        glBindImageTexture(GlobalCom::COLOR_IMAGE_BINDING, textureColors[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(GlobalCom::NORMAL_IMAGE_BINDING, textureNormals[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputScene.render(currentShaderProgram);
    }
}
