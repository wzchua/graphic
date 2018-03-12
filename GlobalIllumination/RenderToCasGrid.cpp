#include "RenderToCasGrid.h"



void RenderToCasGrid::initialize(GLuint numOfGrid, GLuint * textureColors, GLuint * textureNormals)
{

    GLuint dim = 512U >> (numOfGrid - 1); // 1: 512, 2: 256, 3: 128, 4:64, 5:32, 6:16, 7:8, 8:4, 9:2, 10:1
    for (int i = 0; i < numOfGrid; i++) {
        GLuint mipLevel = (numOfGrid - 1 == i) ? 10 - numOfGrid + 1 : 2;
        glGenTextures(1, &textureColors[i]);
        glBindTexture(GL_TEXTURE_3D, textureColors[i]);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexStorage3D(GL_TEXTURE_3D, mipLevel, GL_RGBA8, dim, dim, dim);
        glBindTexture(GL_TEXTURE_3D, 0);

        glGenTextures(1, &textureNormals[i]);
        glBindTexture(GL_TEXTURE_3D, textureNormals[i]);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexStorage3D(GL_TEXTURE_3D, mipLevel, GL_RGBA8, dim, dim, dim);
        glBindTexture(GL_TEXTURE_3D, 0);
    }
    if (hasInitialized) {
        return;
    }
    shader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    shader.generateShader("./Shaders/VoxelizeCasGrid.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();

    for (int i = 0; i < 3; i++) {
        voxelMatrixData[i].viewProjMatrixXY = ortho * voxelViewMatriXY;
        glGenBuffers(1, &voxelMatrixBlockId[i]);
        glBindBuffer(GL_UNIFORM_BUFFER, voxelMatrixBlockId[i]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelizeBlock), &voxelMatrixData[i], GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }    
}

void RenderToCasGrid::run(Scene & inputScene, GLBufferObject<CounterBlock>& ssboCounterSet, glm::mat4 worldToVoxelMat, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct>& ssboLogList, GLuint numOfGrid, GLuint * textureColors, GLuint * textureNormals)
{
    auto & cam = inputScene.cam;
    auto refPos = worldToVoxelMat * glm::vec4(cam.getPosition() - 2.0f * cam.getForward(), 1.0f);
    glm::vec3 newMin, newMax;
    glm::vec3 change = -cam.getForward();

    //level 0
    if (change.y < 0) {
        newMin.y = glm::min(glm::max(refPos.y, 0.0f), 512.0f - 128.0f);
        newMax.y = newMin.y + 128.0f;
    }
    else {
        newMax.y = glm::max(glm::min(refPos.y, 512.0f), 128.0f);
        newMin.y = newMax.y - 128.0f;
    }
    if (change.x < 0) {
        newMin.x = glm::min(glm::max(refPos.x, 0.0f), 512.0f - 128.0f);
        newMax.x = newMin.x + 128.0f;
    }
    else {
        newMax.x = glm::max(glm::min(refPos.x, 512.0f), 128.0f);
        newMin.x = newMax.x - 128.0f;
    }
    if (change.z < 0) {
        newMin.z = glm::min(glm::max(refPos.z, 0.0f), 512.0f - 128.0f);
        newMax.z = newMin.z + 128.0f;
    }
    else {
        newMax.z = glm::max(glm::min(refPos.z, 512.0f), 128.0f);
        newMin.z = newMax.z - 128.0f;
    }
    glm::vec3 translate = -newMin;
    glm::mat4 level0WorldToVoxelMat = glm::translate(glm::mat4(1.0f), translate);
    glm::vec3 voxelMin = level0WorldToVoxelMat * glm::vec4(newMin, 1.0); //should be 0
    glm::vec3 voxelMax = level0WorldToVoxelMat * glm::vec4(newMax, 1.0); //should be 127

    voxelMatrixData[0].worldToVoxelMat = level0WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockId[0], 0, sizeof(VoxelizeBlock), &voxelMatrixData[0]);

    //level 1
    if (change.y < 0) {
        newMin.y = glm::min(glm::max(refPos.y, 0.0f), 512.0f - 256.0f);
        newMax.y = newMin.y + 256.0f;
    }
    else {
        newMax.y = glm::max(glm::min(refPos.y, 512.0f), 256.0f);
        newMin.y = newMax.y - 256.0f;
    }
    if (change.x < 0) {
        newMin.x = glm::min(glm::max(refPos.x, 0.0f), 512.0f - 256.0f);
        newMax.x = newMin.x + 256.0f;
    }
    else {
        newMax.x = glm::max(glm::min(refPos.x, 512.0f), 256.0f);
        newMin.x = newMax.x - 256.0f;
    }
    if (change.z < 0) {
        newMin.z = glm::min(glm::max(refPos.z, 0.0f), 512.0f - 256.0f);
        newMax.z = newMin.z + 256.0f;
    }
    else {
        newMax.z = glm::max(glm::min(refPos.z, 512.0f), 256.0f);
        newMin.z = newMax.z - 256.0f;
    }
    translate = -newMin;
    glm::mat4 level1WorldToVoxelMat = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)), translate) * worldToVoxelMat;
    glm::vec3 voxelMin = level1WorldToVoxelMat * glm::vec4(newMin, 1.0); //should be 0
    glm::vec3 voxelMax = level1WorldToVoxelMat * glm::vec4(newMax, 1.0); //should be 127

    voxelMatrixData[1].worldToVoxelMat = level1WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockId[1], 0, sizeof(VoxelizeBlock), &voxelMatrixData[1]);

    //level 2
    glm::mat4 level2WorldToVoxelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)) * worldToVoxelMat;
    glm::vec3 voxelMin = level2WorldToVoxelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0); //should be 0
    glm::vec3 voxelMax = level2WorldToVoxelMat * glm::vec4(512.0f, 512.0f, 512.0f, 1.0); //should be 127

    voxelMatrixData[2].worldToVoxelMat = level2WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockId[2], 0, sizeof(VoxelizeBlock), &voxelMatrixData[2]);

    GLuint currentShaderProgram = shader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboCounterSet.bind(1);
    ssboLogList.bind(7);

    GLuint dim = 512U >> (numOfGrid - 1); // 1: 512, 2: 256, 3: 128, 4:64, 5:32, 6:16, 7:8, 8:4, 9:2, 10:1

    glViewport(0, 0, dim, dim);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    for (int i = 0; i < numOfGrid; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelMatrixBlockId[i]); //generate new voxelizeblocks
        glBindImageTexture(4, textureColors[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(5, textureNormals[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);;

        inputScene.render(currentShaderProgram);
    }
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

RenderToCasGrid::RenderToCasGrid()
{
}


RenderToCasGrid::~RenderToCasGrid()
{
}
