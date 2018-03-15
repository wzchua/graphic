#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>

#include "Scene.h"
#include "VoxelizeBlock.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "NodeStruct.h"
#include "VoxelVisualizer.h"

#include "RenderToGrid.h"

#include "RenderToOctree.h"
#include "AddToOctree.h"
#include "RenderLightIntoOctree.h"
#include "FilterOctree.h"
#include "RenderVoxelConeTrace.h"

#include "CascadedGrid.h"
#include "RenderToCasGrid.h"
#include "RenderLightIntoCasGrid.h"
#include "FilterCasGrid.h"

class Voxelizer
{
public:
    struct LimitsBlock {
        GLuint maxNoOfFragments;
        GLuint maxNoOfNodes;
        GLuint maxNoOfBricks;
        GLuint maxNoOfLogs;
    };
    enum Type {
        GRID, OCTREE, CAS_GRID
    };
    Voxelizer();
    ~Voxelizer();
    void initializeWithScene(glm::vec3 min, glm::vec3 max);
    void render(Scene& scene);
    void resetAllData();
    int projectionAxis = 0;
private:

    Type mType = CAS_GRID;
    unsigned int fragCount = 1024 * 1024 * 2;
    unsigned int nodeCount = 1024 * 1024 * 2;
    int brickDim = 2;
    int texWdith = 512;
    int texHeight = 512;
    unsigned int maxLogCount = 500;

    VoxelizeBlock voxelMatrixData;
    LimitsBlock voxelLogCountData = { fragCount, nodeCount, nodeCount, maxLogCount };
    const CounterBlock mZeroedCounterBlock = { 0, 1, 0, 0, 0, 0 };
    CounterBlock mCounterBlock = mZeroedCounterBlock;

    GLuint voxelMatrixUniformBuffer;
    GLuint voxelLogUniformBuffer;

    VoxelVisualizer mModuleVoxelVisualizer;

    RenderToOctree mModuleRenderToOctree;
    AddToOctree mModuleAddToOctree;
    FilterOctree mModuleFilterOctree;
    RenderLightIntoOctree mModuleRenderLightIntoOctree;
    RenderVoxelConeTrace mModuleRenderVCT;

    RenderToCasGrid mModuleRenderToCasGrid;
    RenderLightIntoCasGrid mModuleRenderLightIntoCasGrid;
    FilterCasGrid mModuleFilterCasGrid;


    RenderToGrid mModuleRenderToGrid;

    GLuint atomicFragCounterTest;

    GLBufferObject<CounterBlock> ssboCounterSet;
    GLBufferObject<FragStruct> ssboFragmentList;
    GLBufferObject<NodeStruct> ssboNodeList;
    GLBufferObject<LogStruct> ssboLogList;
    GLBufferObject<GLuint> ssboLeafIndexList;
    GLBufferObject<glm::vec4> ssboVoxelList;

    GLuint texture3DColorList;
    GLuint texture3DNormalList;
    GLuint texture3DLightDirList;
    GLuint texture3DLightEnergyList;

    CascadedGrid mCascadedGrid;

    GLuint texture3DColorGrid;
    GLuint texture3DNormalGrid;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;

    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatriXY = glm::lookAt(glm::vec3(256, 256, 0), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriZY = glm::lookAt(glm::vec3(0, 256, 256), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriXZ = glm::lookAt(glm::vec3(256, 0, 256), glm::vec3(256, 256, 256), glm::vec3(0, 0, 1));
    glm::mat4 viewProjMatrixXY;
    glm::mat4 viewProjMatrixZY;
    glm::mat4 viewProjMatrixXZ;
    glm::vec4 newMin;
    glm::vec4 newMax;

    void initialize3DTextures(GLuint &textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

};

