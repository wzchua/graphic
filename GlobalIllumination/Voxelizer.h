#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>

#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "NodeStruct.h"
#include "RenderToGrid.h"
#include "VoxelVisualizer.h"
#include "RenderToOctree.h"
#include "AddToOctree.h"
#include "FilterOctree.h"
#include "RenderLightIntoOctree.h"

class Voxelizer
{
public:
    struct LimitsBlock {
        GLuint maxNoOfFragments;
        GLuint maxNoOfNodes;
        GLuint maxNoOfBricks;
        GLuint maxNoOfLogs;
    };
    struct VoxelMatrixBlock {
        glm::mat4 worldToVoxelMat;
        glm::mat4 viewProjMatrixXY;
        glm::mat4 viewProjMatrixZY;
        glm::mat4 viewProjMatrixXZ;
    };
    Voxelizer();
    ~Voxelizer();
    void initializeWithScene(glm::vec3 min, glm::vec3 max);
    void voxelizeFragmentList(Scene& scene);
    void resetAllData();
    Camera camVoxel = Camera(glm::vec3(256.0f, 32.0f, 128.0f));
    int projectionAxis = 0;
private:
    int getCount(GLBufferObject<GLuint>& counter);
    int getAndResetCount(GLBufferObject<GLuint>& counter, int resetValue = 0);

    unsigned int fragCount = 1024 * 1024 * 2;
    unsigned int nodeCount = 1024 * 1024 * 2;
    int brickDim = 2;
    int texWdith = 512;
    int texHeight = 512;
    unsigned int maxLogCount = 500;

    VoxelMatrixBlock voxelMatrixData;
    LimitsBlock voxelLogCountData = { fragCount, nodeCount, nodeCount, maxLogCount };
    const CounterBlock mZeroedCounterBlock = { 0, 1, 1, 0, 0, 0 };
    CounterBlock mCounterBlock = mZeroedCounterBlock;

    GLuint voxelMatrixUniformBuffer;
    GLuint voxelLogUniformBuffer;

    RenderToOctree mModuleRenderToOctree;
    AddToOctree mModuleAddToOctree;
    FilterOctree mModuleFilterOctree;
    RenderLightIntoOctree mModuleRenderLightIntoOctree;

    RenderToGrid mModuleRenderToGrid;
    VoxelVisualizer mModuleVoxelVisualizer;

    GLuint atomicFragCounterTest;

    GLBufferObject<CounterBlock> ssboCounterSet;
    GLBufferObject<FragStruct> ssboFragmentList;
    GLBufferObject<NodeStruct> ssboNodeList;
    GLBufferObject<LogStruct> ssboLogList;
    GLBufferObject<GLuint> ssboLeafIndexList;
    GLBufferObject<glm::vec4> ssboVoxelList;

    GLuint texture3DColorList;
    GLuint texture3DNormalList;

    GLuint texture3DColorGrid;
    GLuint texture3DNormalGrid;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;

    glm::mat4 worldToVoxelMat;    
    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatriXY = glm::lookAt(glm::vec3(256, 256, 0), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriZY = glm::lookAt(glm::vec3(0, 256, 256), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriXZ = glm::lookAt(glm::vec3(256, 0, 256), glm::vec3(256, 256, 256), glm::vec3(0, 0, 1));
    glm::mat4 viewProjMatrixXY;
    glm::mat4 viewProjMatrixZY;
    glm::mat4 viewProjMatrixXZ;
    glm::vec4 newMin;
    glm::vec4 newMax;

};

