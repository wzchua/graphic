#pragma once
#include <glm/glm.hpp>

struct VoxelizeBlock {
    glm::mat4 worldToVoxelMat;
    glm::mat4 viewProjMatrixXY;
    glm::mat4 worldToVoxelClipmapL0Mat;
    glm::mat4 worldToVoxelClipmapL1Mat;
    glm::mat4 worldToVoxelClipmapL2Mat;
};
static std::string voxelizeBlockString = R"(layout(binding = 0, std140) uniform VoxelizeMatrixBlock {
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY; 
    mat4 worldToVoxelClipmapL0Mat;
    mat4 worldToVoxelClipmapL1Mat;
    mat4 worldToVoxelClipmapL2Mat;
};
)";