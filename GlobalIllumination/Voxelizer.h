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
    };
    struct nodeStruct {
        unsigned int childPtr;
        unsigned int brickPtrX;
        unsigned int parentPtr;
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
    void voxelizeFragmentList(Scene scene);
private:
    void getLogs(std::vector<logStruct> & logs);
    int getCount(GLuint counterId);
    unsigned int count = 1024 * 1024; //1m count
    int brickDim = 3;
    int texWdith = 512;
    int texHeight = 512;
    unsigned int maxLogCount = 500;
    GLuint atomicFragCountPtr;
    GLuint atomicNodeCountPtr;
    GLuint atomicBrickCountPtrX;
    GLuint atomicLogCounter;
    GLuint ssboFragmentList;
    GLuint ssboFragmentList2;
    GLuint ssboNodeList;
    GLuint ssboLogList;
    GLuint texture3DBrickList;
    GLuint texture3DLockList;
    ShaderProgram voxelizeListShader;
    ShaderProgram octreeCompShader;
    glm::mat4 sceneMat;
    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatrix = glm::lookAt(glm::vec3(0, 0, 256), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 modelViewMat;
    glm::mat4 modelViewProjMat;
    glm::vec3 newMin;
    glm::vec3 newMax;
};

