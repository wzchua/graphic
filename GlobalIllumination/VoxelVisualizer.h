#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.h"
#include "GLBufferObject.h"
#include "Camera.h"
#include "LogStruct.h"
#include "CounterBlock.h"
#include "Octree.h"
#include "CascadedGrid.h"
#include "GlobalShaderComponents.h"
class VoxelVisualizer
{
private:
    struct RayCastBlock {
        glm::mat4 viewToVoxelMat;
        glm::vec4 camPosition;
        glm::vec4 camForward;
        glm::vec4 camUp;
        int height;
        int width;
        int isEnergy;
        int gridDef;
    };

    ShaderProgram voxelVisualizerShader;
    GLuint uniformLocModelViewProjMat;

    ShaderProgram voxelRayCastGridShader;
    ShaderProgram voxelRayCastOctreeShader;
    ShaderProgram voxelRayCastCasGridShader;
    GLuint uniformBufferRaycastBlock;

    bool hasInitialized = false;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, int gridType, int gridDef);
public:
    enum Type {
        GRID, OCTREE
    };
    void initialize();
    void rasterizeVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<glm::vec4>& inputssboVoxelList, GLuint noOfVoxels, GLuint colorTextureId);
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, Octree & octree, int gridType);
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, CascadedGrid & cascadedGrid, int gridType, int gridDef);
    void rayCastVoxelsGrid(Camera& cam, glm::mat4 & worldToVoxelMat, GLuint colorTextureId, int gridType);
    VoxelVisualizer();
    ~VoxelVisualizer();
};

