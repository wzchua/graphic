#include "RenderLightIntoCasGrid.h"



void RenderLightIntoCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    shader.generateShader("./Shaders/RSM.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/RSMCasGrid.frag", ShaderProgram::FRAGMENT);
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

void RenderLightIntoCasGrid::run(Scene & inputScene,  GLuint voxelizeMatrixBlock, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct> & logList)
{
    auto& textureLightDirections = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto& textureLightEnergies = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, 1024, 1024);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    shader.use();
    inputScene.updateLightMatrixBuffer(0, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
    ssboCounterSet.bind(1);
    logList.bind(7);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getLightMatrixBuffer()); // light as camera
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, inputScene.getLightBuffer());
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, cascadedGrid.getVoxelizedCascadedBlockBufferId());

    glBindImageTexture(0, textureLightDirections[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, textureLightEnergies[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, textureLightDirections[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, textureLightEnergies[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, textureLightDirections[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, textureLightEnergies[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());
    auto c = ssboCounterSet.getPtr();
    int logCount = c->logCounter;
    c->logCounter = 0;
    ssboCounterSet.unMapPtr();
    std::vector<LogStruct> logs;
    ShaderLogger::getLogs(logList, logCount, logs);
    std::cout << "h\n";

    inputScene.updateLightMatrixBuffer(0, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputScene.render(shader.getProgramId());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
