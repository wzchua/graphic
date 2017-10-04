#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "ShaderProgram.h"
#include "Scene.h"

class Voxelizer
{
public:
    struct fragStruct {
        float position[4];
        float color[4];
        float normal[4];
    };
    struct nodeStruct {
        unsigned int parentPtr;
        unsigned int selfPtr;
        unsigned int childPtr;
        unsigned int modelBrickPtr;
        unsigned int lightBrickPtr;
    };
    struct logStruct {
        float position[4];
        float color[4];
        unsigned int nodeIndex;
        unsigned int brickIndex;
        unsigned int index1;
        unsigned int index2;
    };
    Voxelizer();
    ~Voxelizer();
    void initializeWithScene(glm::vec3 min, glm::vec3 max);
    void voxelizeFragmentList(Scene& scene);
    void resetAllData();
private:
    void getLogs(std::vector<logStruct> & logs, bool reset = false);
    int getCount(GLuint counterId);
    int getAndResetCount(GLuint counterId, int resetValue = 0);
    unsigned int count = 1024 * 1024; //1m count
    int brickDim = 2;
    int texWdith = 512;
    int texHeight = 512;
    unsigned int maxLogCount = 500;

    GLuint atomicFragCountPtr;
    GLuint atomicNodeCountPtr;
    GLuint atomicModelBrickCounterPtr;
    GLuint atomicLeafNodeCountPtr;
    GLuint atomicLogCounter;

    GLuint ssboFragmentList;
    GLuint ssboFragmentList2;
    GLuint ssboNodeList;
    GLuint ssboLeafNodeList;
    GLuint ssboLogList;

    GLuint texture3DrgColorBrickList;
    GLuint texture3DbaColorBrickList;
    GLuint texture3DxyNormalBrickList;
    GLuint texture3DzwNormalBrickList;
    GLuint texture3DCounterList;

    GLuint texture3DColorList;
    GLuint texture3DNormalList;

    ShaderProgram voxelizeListShader;
    ShaderProgram octreeCompShader;
    ShaderProgram octreeAverageCompShader;
    ShaderProgram octreeRenderShader;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;

    glm::mat4 sceneMat;
    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatrix = glm::lookAt(glm::vec3(0, 0, 256), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 modelViewMat;
    glm::mat4 modelViewProjMat;
    glm::vec3 newMin;
    glm::vec3 newMax;
};

