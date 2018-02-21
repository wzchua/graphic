#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.h"
#include "GLBufferObject.h"
#include "Camera.h"
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
    };

    ShaderProgram voxelVisualizerShader;
    GLuint uniformLocModelViewProjMat;

    ShaderProgram voxelRayCastGridShader;
    ShaderProgram voxelRayCastOctreeShader;
    GLuint uniformBufferRaycastBlock;

    bool hasInitialized = false;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;
public:
    enum Type {
        GRID, OCTREE
    };
    void initialize();
    void rasterizeVoxels(Camera& cam, glm::mat4 worldToVoxelMat, GLBufferObject<glm::vec4>& inputssboVoxelList, GLuint noOfVoxels, GLuint colorTextureId);
    void rayCastVoxels(Camera& cam, glm::mat4 worldToVoxelMat, GLuint colorTextureId, Type type);
    VoxelVisualizer();
    ~VoxelVisualizer();
};

