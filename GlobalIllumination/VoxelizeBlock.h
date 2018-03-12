#pragma once
#include <glm/glm.hpp>

struct VoxelizeBlock {
    glm::mat4 worldToVoxelMat;
    glm::mat4 viewProjMatrixXY;
    glm::mat4 viewProjMatrixZY;
    glm::mat4 viewProjMatrixXZ;
};