#include "RenderVoxelConeTraceCasGrid.h"



void RenderVoxelConeTraceCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GenericShaderCodeString::vertHeader << GenericShaderCodeString::vertFragOutput;
    shader.generateShader(vertShaderString, "./Shaders/VoxelConeTracingRender.vert", ShaderProgram::VERTEX);

    std::stringstream fragShaderString;
    fragShaderString << GenericShaderCodeString::fragHeader;
    //uniform blocks
    fragShaderString << GenericShaderCodeString::materialUniformBlock(1) << voxelizeBlockString(3) << voxelizeCascadedBlockString(4);
    fragShaderString << cameraUniformBlockShaderCodeString(5) << GenericShaderCodeString::genericLimitsUniformBlock(7);
    //ssbo
    fragShaderString << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);

    shader.generateShader(fragShaderString, "./Shaders/VoxelConeTracingCasGridRender.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();


    glGenBuffers(1, &camBlkBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, camBlkBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RenderVoxelConeTraceCasGrid::run(Scene & inputScene, GLuint voxelizeMatrixBlock, GLBufferObject<CounterBlock>& ssboCounterSet, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct> & logList)
{
    shader.use();
    inputScene.updateMatrixBuffer();
    auto & res = inputScene.cam.getResolution();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, res.x, res.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
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

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getMatrixBuffer()); //scene cam matrices
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, cascadedGrid.getVoxelizedCascadedBlockBufferId());
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, camBlkBufferId); //scene cam matrices
    ssboCounterSet.bind(1);
    logList.bind(7);

    glMemoryBarrier(GL_UNIFORM_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    inputScene.render(shader.getProgramId());
    auto c = ssboCounterSet.getPtr();
    int logCount = c->logCounter;
    c->logCounter = 0;
    ssboCounterSet.unMapPtr();
    std::vector<LogStruct> logs;
    ShaderLogger::getLogs(logList, logCount, logs);
    //std::cout << "h\n";
}
