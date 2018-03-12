#pragma once
#include <string>
#include <glm/glm.hpp>

struct VoxelizeCascadedBlock {
    glm::mat4 voxelToClipmapL0Mat;
    glm::mat4 voxelToClipmapL1Mat;
    glm::mat4 voxelToClipmapL2Mat;
    glm::vec4 level0min;
    glm::vec4 level0max;
    glm::vec4 level1min;
    glm::vec4 level1max;
    glm::vec4 level2min;
    glm::vec4 level2max;
};
static std::string voxelizeCascadedBlockString(int level) {
    std::string s = R"(layout(binding = )" + std::to_string(level);
    s = s + R"(, std140) uniform VoxelizeCascadedBlock{
    mat4 voxelToClipmapL0Mat;
    mat4 voxelToClipmapL1Mat;
    mat4 voxelToClipmapL2Mat;
    vec4 level0min;
    vec4 level0max;
    vec4 level1min;
    vec4 level1max;
    vec4 level2min;
    vec4 level2max;
};
)";
    return s;
}