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
#include "RenderToFragmentList.h"
#include "RenderToGrid.h"

class Voxelizer
{
public:
    struct nodeStruct {
        unsigned int parentPtr;
        unsigned int selfPtr;
        unsigned int childPtr;
        unsigned int childBit;
        unsigned int modelBrickPtr;
        unsigned int lightBit;
        unsigned int lightBrickPtr;
    };
    struct LimitsBlock {
        GLuint maxNoOfLogs;
        GLuint maxNoOfFragments;
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
    void getLogs(std::vector<LogStruct> & logs, bool reset = false);
    int getCount(GLBufferObject<GLuint>& counter);
    int getAndResetCount(GLBufferObject<GLuint>& counter, int resetValue = 0);

    unsigned int fragCount = 1024 * 1024 * 8;
    unsigned int nodeCount = 1024 * 1024 * 4;
    int brickDim = 2;
    int texWdith = 512;
    int texHeight = 512;
    unsigned int maxLogCount = 500;

    VoxelMatrixBlock voxelMatrixData;
    LimitsBlock voxelLogCountData = { maxLogCount, fragCount };
    const CounterBlock mZeroedCounterBlock = { 0, 0 };
    CounterBlock mCounterBlock = mZeroedCounterBlock;

    GLuint voxelMatrixUniformBuffer;
    GLuint voxelLogUniformBuffer;

    RenderToFragmentList mModuleToFragList;
    RenderToGrid mModuleRenderToGrid;

    GLuint atomicFragCounterTest;

    GLBufferObject<GLuint> atomicFragCounter;
    GLBufferObject<GLuint> atomicNodeCounter;
    GLBufferObject<GLuint> atomicModelBrickCounter;
    GLBufferObject<GLuint> atomicLeafNodeCounter;
    GLBufferObject<GLuint> atomicLogCounter;

    GLBufferObject<CounterBlock> ssboCounterSet;
    GLBufferObject<FragStruct> ssboFragmentList;
    GLBufferObject<FragStruct> ssboFragmentList2;
    GLBufferObject<LogStruct> ssboLogList;
    
    GLuint ssboNodeList;
    GLuint ssboLeafNodeList;

    GLuint texture3DrgColorBrickList;
    GLuint texture3DbaColorBrickList;
    GLuint texture3DxyNormalBrickList;
    GLuint texture3DzwNormalBrickList;
    GLuint texture3DCounterList;

    GLuint texture3DColorList;
    GLuint texture3DNormalList;

    GLuint texture3DColorGrid;
    GLuint texture3DNormalGrid;

    ShaderProgram octreeCompShader;
    ShaderProgram octreeAverageCompShader;
    ShaderProgram octreeRenderShader;

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

