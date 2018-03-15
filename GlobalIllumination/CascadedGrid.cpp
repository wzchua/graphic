#include "CascadedGrid.h"



void CascadedGrid::updateVoxelMatrixBlock(glm::mat4 & worldToVoxelMat, glm::vec4 refPos, glm::vec3 change)
{
    glm::vec3 newMin, newMax;

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
    voxelCascadedData.level0min = glm::vec4(newMin, 1.0f);
    voxelCascadedData.level0max = glm::vec4(newMax, 1.0f);
    voxelCascadedData.voxelToClipmapL0Mat = glm::translate(glm::mat4(1.0f), translate);
    glm::mat4 level0WorldToVoxelMat = voxelCascadedData.voxelToClipmapL0Mat * worldToVoxelMat;
    glm::vec3 voxelMin = level0WorldToVoxelMat * glm::vec4(newMin, 1.0); //should be 0
    glm::vec3 voxelMax = level0WorldToVoxelMat * glm::vec4(newMax, 1.0); //should be 127

    voxelMatrixData[0].worldToVoxelMat = level0WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockIds[0], 0, sizeof(VoxelizeBlock), &voxelMatrixData[0]);

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
    voxelCascadedData.level1min = glm::vec4(newMin, 1.0f);
    voxelCascadedData.level1max = glm::vec4(newMax, 1.0f);
    voxelCascadedData.voxelToClipmapL1Mat = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)), translate);

    glm::mat4 level1WorldToVoxelMat = voxelCascadedData.voxelToClipmapL1Mat * worldToVoxelMat;
    voxelMin = level1WorldToVoxelMat * glm::vec4(newMin, 1.0); //should be 0
    voxelMax = level1WorldToVoxelMat * glm::vec4(newMax, 1.0); //should be 127

    voxelMatrixData[1].worldToVoxelMat = level1WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockIds[1], 0, sizeof(VoxelizeBlock), &voxelMatrixData[1]);

    //level 2
    voxelCascadedData.level2min = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    voxelCascadedData.level2max = glm::vec4(512.0f, 512.0f, 512.0f, 1.0f);
    voxelCascadedData.voxelToClipmapL2Mat = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));

    glm::mat4 level2WorldToVoxelMat = voxelCascadedData.voxelToClipmapL2Mat * worldToVoxelMat;
    voxelMin = level2WorldToVoxelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0); //should be 0
    voxelMax = level2WorldToVoxelMat * glm::vec4(512.0f, 512.0f, 512.0f, 1.0); //should be 127

    voxelMatrixData[2].worldToVoxelMat = level2WorldToVoxelMat;
    glNamedBufferSubData(voxelMatrixBlockIds[2], 0, sizeof(VoxelizeBlock), &voxelMatrixData[2]);

    glNamedBufferSubData(voxelCascadedBlockId, 0, sizeof(VoxelizeCascadedBlock), &voxelCascadedData);
}

bool CascadedGrid::isWithinBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max)
{
    return (min.x <= pos.x && pos.x <= max.x) && (min.y <= pos.y && pos.y <= max.y) && (min.z <= pos.z && pos.z <= max.z);
}

bool CascadedGrid::isOutsideBoundaries(glm::vec3 pos, glm::vec3 min, glm::vec3 max)
{
    return pos.x < min.x || max.x < pos.x || pos.y < min.y || max.y < pos.y || pos.z < min.z || max.z < pos.z;
}

void CascadedGrid::initializeGrids(GLuint cascadeNumber)
{
    if (hasInitialized) {
        return;
    }
    hasInitialized = true;

    mCascadeNumber = cascadeNumber;
    dim = 512U >> (cascadeNumber - 1); // 1: 512, 2: 256, 3: 128, 4:64, 5:32, 6:16, 7:8, 8:4, 9:2, 10:1

    texture3DColorCasGrid.resize(cascadeNumber);
    texture3DNormalCasGrid.resize(cascadeNumber);
    texture3DLightDirCasGrid.resize(cascadeNumber);
    texture3DLightEnergyCasGrid.resize(cascadeNumber);

    voxelMatrixBlockIds.resize(cascadeNumber);
    voxelMatrixData.resize(cascadeNumber);

    for (int i = 0; i < cascadeNumber; i++) {
        GLuint mipLevel = (cascadeNumber - 1 == i) ? 10 - cascadeNumber + 1 : 1;
        
        GLuint & textureId = texture3DColorCasGrid[i];

        glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
        glTextureStorage3D(textureId, mipLevel, GL_RGBA8, dim, dim, dim);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        textureId = texture3DNormalCasGrid[i];

        glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
        glTextureStorage3D(textureId, mipLevel, GL_RGBA8, dim, dim, dim);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        textureId = texture3DLightDirCasGrid[i];

        glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
        glTextureStorage3D(textureId, mipLevel, GL_RGBA8, dim, dim, dim);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        textureId = texture3DLightEnergyCasGrid[i];

        glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
        glTextureStorage3D(textureId, mipLevel, GL_R32UI, dim, dim, dim);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glGenBuffers(1, &voxelCascadedBlockId);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelCascadedBlockId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelizeCascadedBlock), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (int i = 0; i < 3; i++) {
        voxelMatrixData[i].viewProjMatrixXY = ortho * voxelViewMatriXY;
        glGenBuffers(1, &voxelMatrixBlockIds[i]);
        glBindBuffer(GL_UNIFORM_BUFFER, voxelMatrixBlockIds[i]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelizeBlock), &voxelMatrixData[i], GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

GLuint CascadedGrid::getClipDimensions()
{
    return dim;
}

GLuint CascadedGrid::getCascadedLevels()
{
    return mCascadeNumber;
}

std::vector<GLuint> & CascadedGrid::getVoxelMatrixBlockIds()
{
    return voxelMatrixBlockIds;
}

glm::mat4 CascadedGrid::getWorldToVoxelClipmapMatrix(GLuint level)
{
    return voxelMatrixData[level].worldToVoxelMat;
}

glm::mat4 CascadedGrid::getWorldToVoxelClipmapMatrixFromPos(glm::vec3 pos, GLuint & outLevel)
{
    if (isOutsideBoundaries(pos, voxelCascadedData.level1min, voxelCascadedData.level1max)) {
        outLevel = 2;
        return voxelMatrixData[2].worldToVoxelMat;
    }
    else if (isOutsideBoundaries(pos, voxelCascadedData.level0min, voxelCascadedData.level0max)) {
        outLevel = 1;
        return voxelMatrixData[1].worldToVoxelMat;
    }
    return voxelMatrixData[0].worldToVoxelMat;
}

void CascadedGrid::setRefCamPosition(glm::vec4 refPos, glm::mat4 & worldToVoxelMat, glm::vec3 camForward)
{
    if (refPos != refCamPosCache) {
        refCamPosCache =refPos;
        updateVoxelMatrixBlock(worldToVoxelMat, refPos, camForward);
    }
}

std::vector<GLuint>& CascadedGrid::getCasGridTextureIds(GridType type)
{
    switch (type) {
    case COLOR:
        return texture3DColorCasGrid;
    case NORMAL:
        return texture3DNormalCasGrid;
    case LIGHT_DIRECTION:
        return texture3DLightDirCasGrid;
    case LIGHT_ENERGY:
        return texture3DLightEnergyCasGrid;
    default:
        throw new std::invalid_argument("Invalid GridType");
    }
}

VoxelizeCascadedBlock & CascadedGrid::getVoxelizedCascadedBlock()
{
    return voxelCascadedData;
}

GLuint CascadedGrid::getVoxelizedCascadedBlockBufferId()
{
    return voxelCascadedBlockId;
}

CascadedGrid::CascadedGrid()
{
}


CascadedGrid::~CascadedGrid()
{
}
