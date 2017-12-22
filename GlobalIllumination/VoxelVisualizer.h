#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.h"
#include "GLBufferObject.h"
#include "Camera.h"
class VoxelVisualizer
{
private:

    ShaderProgram voxelVisualizerShader;
    bool hasInitialized = false;
public:
    void initialize();
    void run(Camera& cam, glm::mat4 worldToVoxelMat, GLBufferObject& inputssboVoxelList, GLuint noOfVoxels);
    VoxelVisualizer();
    ~VoxelVisualizer();
};

