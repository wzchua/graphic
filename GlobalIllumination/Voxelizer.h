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
        unsigned int brickPtrY;
    };
    struct syncStruct {
        unsigned int treadId;
    };
    Voxelizer();
    ~Voxelizer();
    void initializeWithScene(glm::vec3 min, glm::vec3 max);
    void voxelizeFragmentList(Scene scene);
private:
    unsigned int count = 512 * 256 * 256;
    int brickDim = 3;
    int texWdith = 400;
    int texHeight = 400;
    GLuint atomicNodeCountPtr;
    GLuint atomicBrickCountPtrX;
    GLuint atomicBrickCountPtrY;
    GLuint ssboFragmentList;
    GLuint ssboNodeList;
    GLuint ssboSyncList;
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

