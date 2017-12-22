#pragma once
#include <glm/gtc/type_ptr.hpp>
#include "GLBufferObject.h"
#include "ShaderProgram.h"
#include "Scene.h"
class RenderToFragmentList
{
private:
    struct logStruct {
        float position[4];
        float color[4];
    };
    GLBufferObject atomicLogCounter;
    GLBufferObject ssboLogList;
    ShaderProgram voxelizeListShader;

    glm::mat4 worldToVoxelMat;
    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatriXY = glm::lookAt(glm::vec3(256, 256, 0), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriZY = glm::lookAt(glm::vec3(0, 256, 256), glm::vec3(256, 256, 256), glm::vec3(0, 1, 0));
    glm::mat4 voxelViewMatriXZ = glm::lookAt(glm::vec3(256, 0, 256), glm::vec3(256, 256, 256), glm::vec3(0, 0, 1));
    glm::mat4 viewProjMatrixXY;
    glm::mat4 viewProjMatrixZY;
    glm::mat4 viewProjMatrixXZ;
    glm::vec3 newMin;
    glm::vec3 newMax;

    bool hasInitialized = false;
    unsigned int maxLogCount = 500;
public:
    int projectionAxis = 0;
    void initialize(GLBufferObject & atomicFragCounter, GLBufferObject & ssboFragList, GLsizeiptr sizeFragStruct);
    void run(Scene& scene, GLBufferObject & atomicFragCounter, GLBufferObject & ssboFragList);
    RenderToFragmentList();
    ~RenderToFragmentList();
};

