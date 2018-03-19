#include "Octree.h"



void Octree::initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
    glTextureStorage3D(textureId, levels, internalformat, width, height, depth);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Octree::initialize()
{
    ssboNodeList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(NodeStruct) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboLeafIndexList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);

    initialize3DTextures(texture3DColorList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DNormalList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DLightDirList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DLightEnergyList, 2, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
}

void Octree::resetData()
{

    ssboNodeList.clearData();
    /*
    auto node = ssboNodeList.getPtr();
    std::vector<NodeStruct> nodeList;
    for (int i = 0; i < 50; i++) {
        nodeList.push_back(node[i]);
    }
    ssboNodeList.unMapPtr();*/

    glInvalidateTexImage(texture3DColorList, 0);
    glClearTexImage(texture3DColorList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalList, 0);
    glClearTexImage(texture3DNormalList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DLightEnergyList, 0);
    glClearTexImage(texture3DLightEnergyList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DLightDirList, 0);
    glClearTexImage(texture3DLightDirList, 0, GL_RGBA, GL_FLOAT, NULL);
}

GLBufferObject<Octree::NodeStruct>& Octree::getNodeList()
{
    return ssboNodeList;
}

GLBufferObject<GLuint>& Octree::getLeafIndexList()
{
    return ssboLeafIndexList;
}

GLuint Octree::getTextureIds(TexType type)
{
    switch (type) {
    case COLOR:
        return texture3DColorList;
    case NORMAL:
        return texture3DNormalList;
    case LIGHT_DIRECTION:
        return texture3DLightDirList;
    case LIGHT_ENERGY:
        return texture3DLightEnergyList;
    default:
        throw new std::invalid_argument("Invalid TexType Octree");
    }
}

Octree::Octree()
{
}


Octree::~Octree()
{
}
