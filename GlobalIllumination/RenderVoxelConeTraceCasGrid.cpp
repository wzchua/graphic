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
    auto & res = inputScene.cam.getResolution();
    GLuint output = gBuffer.getAdditionalBuffers(GBuffer::INDIRECT);

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
    glm::ivec2 setOfWorkGroups = glm::ivec2(ceil(res.x * 1.0f / mWorkGroupSize.x), ceil(res.y * 1.0f / mWorkGroupSize.y));
    glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, 1);
    /*auto c = ssboCounterSet.getPtr();
    int logCount = c->logCounter;
    c->logCounter = 0;
    ssboCounterSet.unMapPtr();
    std::vector<LogStruct> logs;
    ShaderLogger::getLogs(logList, logCount, logs);
    std::cout << "h\n";
    gBuffer.dumpBuffersAsImages();*/
}
