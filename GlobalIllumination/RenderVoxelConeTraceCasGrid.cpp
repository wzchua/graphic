#include "RenderVoxelConeTraceCasGrid.h"



void RenderVoxelConeTraceCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    compShaderString << voxelizeBlockString(GlobalShaderComponents::VOXELIZATION_MATRIX_UBO_BINDING) << voxelizeCascadedBlockString(GlobalShaderComponents::CASGRID_VOXELIZATION_INFO_UBO_BINDING);
    compShaderString << cameraUniformBlockShaderCodeString(GlobalCom::CAMERA_UBO_BINDING);
    compShaderString << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    shader.generateShader(compShaderString, "./Shaders/VoxelConeTracingCasGrid.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();


    glGenBuffers(1, &camBlkBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, camBlkBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RenderVoxelConeTraceCasGrid::run(Scene & inputScene, GLBufferObject<CounterBlock>& ssboCounterSet, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct> & logList, GBuffer & gBuffer)
{
    shader.use();
    inputScene.updateMatrixBuffer();
    auto & res = inputScene.cam.getResolution();
    GLuint output = gBuffer.getAdditionalBuffers(0);
    glViewport(0, 0, res.x, res.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    camBlk.camPosition = glm::vec4(inputScene.cam.getPosition(), 1.0f);
    camBlk.camForward = glm::vec4(inputScene.cam.getForward(), 1.0f);
    camBlk.camUp = glm::vec4(inputScene.cam.getUp(), 1.0f);
    camBlk.height = res.y;
    camBlk.width = res.x;
    glNamedBufferSubData(camBlkBufferId, 0, sizeof(CameraBlock), &camBlk);
    
    auto & colorCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto & normalCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);
    auto & lightDirCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto & lightEnergyCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);
    
    for (int i = 0; i < 3; i++) {
        glBindTextureUnit(4 * i + 0, colorCasGrid[i]);
        glBindTextureUnit(4 * i + 1, normalCasGrid[i]);
        glBindTextureUnit(4 * i + 2, lightDirCasGrid[i]);
        glBindTextureUnit(4 * i + 3, lightEnergyCasGrid[i]);
    }
    gBuffer.bindGBuffersAsTexture(12, 13, 14, 15);

    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalCom::CAMERA_MATRIX_UBO_BINDING, inputScene.getMatrixBuffer()); //scene cam matrices
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalCom::CASGRID_VOXELIZATION_INFO_UBO_BINDING, cascadedGrid.getVoxelizedCascadedBlockBufferId());
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalCom::CAMERA_UBO_BINDING, camBlkBufferId); //scene cam matrices

    glBindImageTexture(0, output, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glMemoryBarrier(GL_UNIFORM_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    glDispatchCompute(res.x / mWorkGroupSize.x, res.y / mWorkGroupSize.y, 1);
    auto c = ssboCounterSet.getPtr();
    int logCount = c->logCounter;
    c->logCounter = 0;
    ssboCounterSet.unMapPtr();
    std::vector<LogStruct> logs;
    ShaderLogger::getLogs(logList, logCount, logs);
    std::cout << "h\n";
    gBuffer.dumpBuffersAsImages();
}
