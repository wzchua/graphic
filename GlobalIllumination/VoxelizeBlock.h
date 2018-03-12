#pragma once
#include <string>
#include <glm/glm.hpp>

struct VoxelizeBlock {
    glm::mat4 worldToVoxelMat;
    glm::mat4 viewProjMatrixXY;
};
static std::string voxelizeBlockString(int level) {
    std::string s = R"(layout(binding = )" + std::to_string(level);
    s = s + R"(, std140) uniform VoxelizeMatrixBlock{
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY;
};
)";
    return s;
}