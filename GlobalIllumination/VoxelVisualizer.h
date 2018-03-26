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
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock> & counterSet, GLuint logUniformBlock, GLBufferObject<LogStruct> & logList, int gridType);
public:
    enum Type {
        GRID, OCTREE
    };
    void initialize();
    void rasterizeVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<glm::vec4>& inputssboVoxelList, GLuint noOfVoxels, GLuint colorTextureId);
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock> & counterSet, GLuint logUniformBlock, Octree & octree, GLBufferObject<LogStruct> & logList, int gridType);
    void rayCastVoxels(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock> & counterSet, GLuint logUniformBlock, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct> & logList, int gridType);
    void rayCastVoxelsGrid(Camera& cam, glm::mat4 & worldToVoxelMat, GLBufferObject<CounterBlock> & counterSet, GLuint logUniformBlock, GLuint colorTextureId, GLBufferObject<LogStruct> & logList, int gridType);
    VoxelVisualizer();
    ~VoxelVisualizer();
};

