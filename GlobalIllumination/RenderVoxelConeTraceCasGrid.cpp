#include "RenderVoxelConeTraceCasGrid.h"



void RenderVoxelConeTraceCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    shader.generateShader("./Shaders/VoxelConeTracingRender.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/VoxelConeTracingCasGridRender.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();

    //cube vao
    std::vector<glm::vec3> quadVertices;
    quadVertices.push_back(glm::vec3(1.0, 1.0, 0.0));
    quadVertices.push_back(glm::vec3(1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, 1.0, 0.0));

    std::vector<GLuint> quadindices;
    quadindices.push_back(0);
    quadindices.push_back(3);
    quadindices.push_back(1);
    quadindices.push_back(2);
    quadindices.push_back(1);
    quadindices.push_back(3);

    int vertexSize = sizeof(glm::vec3);
    glGenVertexArrays(1, &quadVAOId);
    glBindVertexArray(quadVAOId);

    glGenBuffers(1, &quadVBOId);
    glGenBuffers(1, &quadEBOId);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBOId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadindices.size() * sizeof(GLuint), quadindices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * vertexSize, quadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RenderVoxelConeTraceCasGrid::run(Scene & inputScene, GLBufferObject<CounterBlock>& ssboCounterSet, CascadedGrid & cascadedGrid)
{
    shader.use();
    glViewport(0, 0, 800, 600); // light render is done at 1024x1024
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
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
    ssboCounterSet.bind(1);

    inputScene.render(shader.getProgramId());
}

RenderVoxelConeTraceCasGrid::RenderVoxelConeTraceCasGrid()
{
}


RenderVoxelConeTraceCasGrid::~RenderVoxelConeTraceCasGrid()
{
}
