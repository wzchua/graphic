#include "VoxelVisualizer.h"



void VoxelVisualizer::initialize()
{
    if (hasInitialized) {
        return;
    }
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

    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.vert", ShaderProgram::VERTEX);
    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.geom", ShaderProgram::GEOMETRY);
    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.frag", ShaderProgram::FRAGMENT);
    voxelVisualizerShader.linkCompileValidate();

    voxelRayCastGridShader.generateShader("./Shaders/VoxelGridRayCast.vert", ShaderProgram::VERTEX);
    voxelRayCastGridShader.generateShader("./Shaders/VoxelGridRayCast.frag", ShaderProgram::FRAGMENT);
    voxelRayCastGridShader.linkCompileValidate();

    voxelRayCastOctreeShader.generateShader("./Shaders/VoxelOctreeRayCast.vert", ShaderProgram::VERTEX);
    voxelRayCastOctreeShader.generateShader("./Shaders/VoxelOctreeRayCast.frag", ShaderProgram::FRAGMENT);
    voxelRayCastOctreeShader.linkCompileValidate();

    voxelRayCastCasGridShader.generateShader("./Shaders/VoxelGridRayCast.vert", ShaderProgram::VERTEX);
    voxelRayCastCasGridShader.generateShader("./Shaders/VoxelCasGridRayCast.frag", ShaderProgram::FRAGMENT);
    voxelRayCastCasGridShader.linkCompileValidate();

    
    glGenBuffers(1, &uniformBufferRaycastBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferRaycastBlock);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RayCastBlock), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void VoxelVisualizer::rasterizeVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<glm::vec4>& inputssboVoxelList, GLuint noOfVoxels, GLuint colorTextureId)
{
    GLuint currentShaderProgram = voxelVisualizerShader.use();

    glm::mat4 modelViewMat = cam.getViewMatrix() * glm::inverse(worldToVoxelMat);
    glm::mat4 modelViewProjMat = cam.getProjMatrix() * modelViewMat;

    glUniformMatrix4fv(uniformLocModelViewProjMat, 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glBindImageTexture(4, colorTextureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    
    int width = 800;
    int height = 600;

    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);


    glBindBuffer(GL_ARRAY_BUFFER, inputssboVoxelList.getId());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_POINTS, 0, noOfVoxels);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, Octree & octree, int gridType)
{
    voxelRayCastOctreeShader.use();
    octree.getNodeList().bind(2);
    octree.getNodeValueList().bind(3);
    rayCastVoxels(cam, worldToVoxelMat, gridType, 0);
}

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, CascadedGrid & cascadedGrid, int gridType, int gridDef)
{
    voxelRayCastCasGridShader.use();
    CascadedGrid::GridType type;
    switch (gridType) {
    case 1:
        type = CascadedGrid::COLOR;
        break;
    case 2:
        type = CascadedGrid::NORMAL;
        break;
    case 3:
        type = CascadedGrid::LIGHT_DIRECTION;
        break;
    case 4:
        type = CascadedGrid::LIGHT_ENERGY;
        break;
    }
    auto& grid = cascadedGrid.getCasGridTextureIds(type);
    glBindTextureUnit(0, grid[0]);
    glBindTextureUnit(1, grid[1]);
    glBindTextureUnit(2, grid[2]);    
    auto& energy = cascadedGrid.getCasGridTextureIds(CascadedGrid::LIGHT_ENERGY);
    glBindTextureUnit(4, energy[0]);
    glBindTextureUnit(5, energy[1]);
    glBindTextureUnit(6, energy[2]);

    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::CASGRID_VOXELIZATION_INFO_UBO_BINDING, cascadedGrid.getVoxelizedCascadedBlockBufferId());
    rayCastVoxels(cam, worldToVoxelMat, gridType, gridDef);
}

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, int gridType, int gridDef)
{
    int width = 800;
    int height = 600;
    glm::mat4 viewToVoxelMat = worldToVoxelMat * glm::inverse(cam.getViewMatrix());
    RayCastBlock block = { viewToVoxelMat, worldToVoxelMat * glm::vec4(cam.getPosition(), 1.0f), glm::vec4(cam.getForward(), 1.0f), glm::vec4(cam.getUp(), 1.0f), height, width, (gridType == 4) ? 1:0, gridDef };
    glNamedBufferSubData(uniformBufferRaycastBlock, 0, sizeof(RayCastBlock), &block);
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::RAYCAST_UBO_BINDING, uniformBufferRaycastBlock);

    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glBindVertexArray(quadVAOId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void VoxelVisualizer::rayCastVoxelsGrid(Camera & cam, glm::mat4 & worldToVoxelMat, GLuint colorTextureId, int gridType)
{
    voxelRayCastGridShader.use();
    glBindImageTexture(4, colorTextureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    rayCastVoxels(cam, worldToVoxelMat, gridType, 0);
}

VoxelVisualizer::VoxelVisualizer()
{
}


VoxelVisualizer::~VoxelVisualizer()
{
}
