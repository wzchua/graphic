#include "VoxelVisualizer.h"



void VoxelVisualizer::initialize()
{
    if (hasInitialized) {
        return;
    }

    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.vert", ShaderProgram::VERTEX);
    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.geom", ShaderProgram::GEOMETRY);
    voxelVisualizerShader.generateShader("./Shaders/VoxelVisualizer.frag", ShaderProgram::FRAGMENT);
    voxelVisualizerShader.linkCompileValidate();
}

void VoxelVisualizer::run(Camera& cam, glm::mat4 worldToVoxelMat, GLBufferObject<glm::ivec4>& inputssboVoxelList, GLuint noOfVoxels)
{
    GLuint currentShaderProgram = voxelVisualizerShader.use();

    glm::mat4 modelViewMat = cam.getViewMatrix() * worldToVoxelMat;
    glm::mat4 modelViewProjMat = cam.getProjMatrix() * modelViewMat;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));

    int width = 800;
    int height = 600;
    glUniform1i(glGetUniformLocation(currentShaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(currentShaderProgram, "height"), height);

    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);


    glBindBuffer(GL_ARRAY_BUFFER, inputssboVoxelList.getId());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::ivec4), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_POINTS, 0, noOfVoxels);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VoxelVisualizer::VoxelVisualizer()
{
}


VoxelVisualizer::~VoxelVisualizer()
{
}
