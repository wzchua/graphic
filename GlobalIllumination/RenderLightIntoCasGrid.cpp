#include "RenderLightIntoCasGrid.h"



void RenderLightIntoCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    shader.generateShader("./Shaders/RSM.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/RSMCasGrid.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
}

void RenderLightIntoCasGrid::run(Scene & inputScene, GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid)
{
    auto& textureLightDirections = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto& textureLightEnergies = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);

    glViewport(0, 0, 1024, 1024); // light render is done at 1024x1024
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    shader.use();
    inputScene.updateLightMatrixBuffer(0, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
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

    inputScene.render(shader.getProgramId());


    inputScene.updateLightMatrixBuffer(0, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
    inputScene.render(shader.getProgramId());
    inputScene.updateLightMatrixBuffer(0, glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
    inputScene.render(shader.getProgramId());

}

RenderLightIntoCasGrid::RenderLightIntoCasGrid()
{
}


RenderLightIntoCasGrid::~RenderLightIntoCasGrid()
{
}
