#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "LogStruct.h"
#include "CounterBlock.h"
#include "VoxelizeBlock.h"
#include "VoxelizeCascadedBlock.h"
class RenderToCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;
    glm::mat4 ortho = glm::ortho(-64.0, 64.0, -64.0, 64.0, 0.0, 128.0);
    glm::mat4 voxelViewMatriXY = glm::lookAt(glm::vec3(64, 64, 0), glm::vec3(64, 64, 64), glm::vec3(0, 1, 0));
    VoxelizeBlock voxelMatrixData[3];
    GLuint voxelMatrixBlockId[3];
    VoxelizeCascadedBlock voxelCascadedData;
    GLuint voxelCascadedBlockId;
    mutable glm::vec4 refPosCache = glm::vec4(-1.0f); //not valid in voxel space 0 to 512
    void updateVoxelMatrixBlock(glm::mat4 & worldToVoxelMat, glm::vec4 refPos, glm::vec3 change);
    bool isWithinBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max);
    bool isOutsideBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max);
public:
    void initialize(GLuint numOfGrid, GLuint * textureColors, GLuint * textureNormals);
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, glm::mat4 & worldToVoxelMat, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint numOfGrid, GLuint * textureColors, GLuint * textureNormals);
    glm::mat4 getWorldToVoxelClipmapMatrix(GLuint level);
    glm::mat4 getWorldToVoxelClipmapMatrixFromPos(glm::vec3 pos, GLuint & outLevel);
    VoxelizeCascadedBlock & getVoxelizedCascadedBlock();
    GLuint getVoxelizedCascadedBlockBufferId();
    RenderToCasGrid();
    ~RenderToCasGrid();
};

