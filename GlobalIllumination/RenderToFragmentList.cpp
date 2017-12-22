#include "RenderToFragmentList.h"



void RenderToFragmentList::initialize(GLBufferObject & atomicFragCounter, GLBufferObject & ssboFragList, GLsizeiptr sizeFragStruct)
{
    if (hasInitialized) {
        return;
    }

    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();


    GLuint zero = 0;
    atomicFragCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, true);
    atomicLogCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, true);

    ssboFragList.initialize(GL_SHADER_STORAGE_BUFFER, sizeFragStruct, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, true);
    ssboLogList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(logStruct) * maxLogCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, true);
}

void RenderToFragmentList::run(Scene & inputScene, GLBufferObject & atomicFragCounter, GLBufferObject & ssboFragList)
{
    auto max = inputScene.getSceneMaxCoords();
    auto min = inputScene.getSceneMinCoords();
    //scale/translate scene into ortho box 512 x 512 x 512
    glm::vec3 length = (max - min);
    glm::vec3 translate = glm::vec3(0.0f) - min;
    glm::vec3 scale = 512.0f / length;
    //fixed ratio scaling
    float smallestScale = glm::min(scale.x, glm::min(scale.y, scale.z));
    scale = glm::vec3(smallestScale);
    worldToVoxelMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    newMin = worldToVoxelMat * glm::vec4(min, 1.0);
    newMax = worldToVoxelMat * glm::vec4(max, 1.0);

    viewProjMatrixXY = ortho * voxelViewMatriXY;
    viewProjMatrixZY = ortho * voxelViewMatriZY;
    viewProjMatrixXZ = ortho * voxelViewMatriXZ;

    GLuint currentShaderProgram = voxelizeListShader.use();
    atomicFragCounter.bind(0);
    atomicLogCounter.bind(7);

    ssboFragList.bind(0);
    ssboLogList.bind(7);


    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(worldToVoxelMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixXY"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixXY));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixZY"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixZY));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixXZ"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixXZ));
    glUniform1i(glGetUniformLocation(currentShaderProgram, "projectionAxis"), projectionAxis);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    inputScene.render(currentShaderProgram);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

RenderToFragmentList::RenderToFragmentList()
{
}


RenderToFragmentList::~RenderToFragmentList()
{
}
