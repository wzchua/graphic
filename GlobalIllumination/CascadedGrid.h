#pragma once
#include <glad\glad.h>
#include <vector>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VoxelizeBlock.h"
#include "VoxelizeCascadedBlock.h"

class CascadedGrid
{
private:
    GLuint mCascadeNumber = 3;
    GLuint dim;
    bool hasInitialized = false;

    std::vector<GLuint> texture3DColorCasGrid;
    std::vector<GLuint> texture3DNormalCasGrid;
    std::vector<GLuint> texture3DLightDirCasGrid;
    std::vector<GLuint> texture3DLightEnergyCasGrid;

    glm::mat4 ortho = glm::ortho(-64.0, 64.0, -64.0, 64.0, 0.0, 128.0);
    glm::mat4 voxelViewMatriXY = glm::lookAt(glm::vec3(64, 64, 0), glm::vec3(64, 64, 64), glm::vec3(0, 1, 0));
    std::vector<VoxelizeBlock> voxelMatrixData;
    std::vector<GLuint> voxelMatrixBlockIds;
    VoxelizeCascadedBlock voxelCascadedData;
    GLuint voxelCascadedBlockId;
    mutable glm::vec4 refCamPosCache = glm::vec4(-1.0f); //not valid in voxel space 0 to 512
    void updateVoxelMatrixBlock(glm::mat4 & worldToVoxelMat, glm::vec4 refPos, glm::vec3 change);
    bool isWithinBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max);
    bool isOutsideBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max);
public:
    enum GridType { COLOR, NORMAL, LIGHT_DIRECTION, LIGHT_ENERGY };
    void initializeGrids(GLuint cascadeNumber); 
    GLuint getClipDimensions();
    GLuint getCascadedLevels();
    std::vector<GLuint> & getVoxelMatrixBlockIds();
    glm::mat4 getWorldToVoxelClipmapMatrix(GLuint level);
    glm::mat4 getWorldToVoxelClipmapMatrixFromPos(glm::vec3 pos, GLuint & outLevel);
    void setRefCamPosition(glm::vec4 refPos, glm::mat4 & worldToVoxelMat, glm::vec3 camForward);
    std::vector<GLuint> & getCasGridTextureIds(GridType type);
    VoxelizeCascadedBlock & getVoxelizedCascadedBlock();
    GLuint getVoxelizedCascadedBlockBufferId();
    CascadedGrid();
    ~CascadedGrid();
};
