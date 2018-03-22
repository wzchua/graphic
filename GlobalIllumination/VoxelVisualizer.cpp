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
    uniformLocModelViewProjMat = glGetUniformLocation(voxelVisualizerShader.getProgramId(), "ModelViewProjMat");

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

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock>& counterSet, GLuint logUniformBlock, Octree & octree, GLBufferObject<LogStruct>& logList)
{
    voxelRayCastOctreeShader.use();
    octree.getNodeList().bind(2);
    octree.getNodeValueList().bind(3);
    rayCastVoxels(cam, worldToVoxelMat, counterSet, logUniformBlock, logList);
}

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock>& counterSet, GLuint logUniformBlock, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct>& logList)
{
    voxelRayCastCasGridShader.use();
    auto& color = cascadedGrid.getCasGridTextureIds(CascadedGrid::COLOR);
    glBindTextureUnit(0, color[0]);
    glBindTextureUnit(1, color[1]);
    glBindTextureUnit(2, color[2]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, cascadedGrid.getVoxelizedCascadedBlockBufferId());
    rayCastVoxels(cam, worldToVoxelMat, counterSet, logUniformBlock, logList);
}

void VoxelVisualizer::rayCastVoxels(Camera & cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock> & counterSet, GLuint logUniformBlock, GLBufferObject<LogStruct> & logList)
{
    int width = 800;
    int height = 600;
    glm::mat4 viewToVoxelMat = worldToVoxelMat * glm::inverse(cam.getViewMatrix());
    RayCastBlock block = { viewToVoxelMat, worldToVoxelMat * glm::vec4(cam.getPosition(), 1.0f), glm::vec4(cam.getForward(), 1.0f), glm::vec4(cam.getUp(), 1.0f), height, width };
    glNamedBufferSubData(uniformBufferRaycastBlock, 0, sizeof(RayCastBlock), &block);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBufferRaycastBlock);
    counterSet.bind(1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);


    logList.bind(7);

    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBindVertexArray(quadVAOId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    auto c = counterSet.getPtr();
    int logCount = c->logCounter;
    c->logCounter = 0;
    counterSet.unMapPtr();
    std::vector<LogStruct> logs;
    ShaderLogger::getLogs(logList, logCount, logs);
    std::cout << "\n";
}

void VoxelVisualizer::rayCastVoxelsGrid(Camera & cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock>& counterSet, GLuint logUniformBlock, GLuint colorTextureId, GLBufferObject<LogStruct>& logList)
{
    voxelRayCastGridShader.use();
    glBindImageTexture(4, colorTextureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    rayCastVoxels(cam, worldToVoxelMat, counterSet, logUniformBlock, logList);
}

VoxelVisualizer::VoxelVisualizer()
{
}


VoxelVisualizer::~VoxelVisualizer()
{
}
