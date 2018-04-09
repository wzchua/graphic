#include "Octree.h"


/*
void Octree::initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
    glTextureStorage3D(textureId, levels, internalformat, width, height, depth);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
*/

void Octree::initialize()
{
    //generateOctree(7);
    ssboNodeList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(NodeStruct) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT, 0);
    ssboNodeValueList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(NodeValueStruct) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboLeafIndexList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboFragList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(FragStruct) * fragCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    
    //glNamedBufferSubData(ssboNodeList.getId(), 0, pregeneratedInitialOctree.size() * sizeof(NodeStruct), pregeneratedInitialOctree.data());
    /*
    initialize3DTextures(texture3DColorList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DNormalList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DLightDirList, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    initialize3DTextures(texture3DLightEnergyList, 2, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    */
}

void Octree::resetData()
{

    ssboNodeList.clearData();
    //(ssboNodeList.getId(), 0, pregeneratedInitialOctree.size() * sizeof(NodeStruct), pregeneratedInitialOctree.data());
    ssboNodeValueList.clearData();
    /*
    auto node = ssboNodeList.getPtr();
    std::vector<NodeStruct> nodeList;
    for (int i = 0; i < 50; i++) {
        nodeList.push_back(node[i]);
    }
    ssboNodeList.unMapPtr();*/

    /*
    glInvalidateTexImage(texture3DColorList, 0);
    glClearTexImage(texture3DColorList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalList, 0);
    glClearTexImage(texture3DNormalList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DLightEnergyList, 0);
    glClearTexImage(texture3DLightEnergyList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DLightDirList, 0);
    glClearTexImage(texture3DLightDirList, 0, GL_RGBA, GL_FLOAT, NULL);
    */
}

GLBufferObject<Octree::NodeStruct>& Octree::getNodeList()
{
    return ssboNodeList;
}
GLBufferObject<Octree::NodeValueStruct>& Octree::getNodeValueList()
{
    return ssboNodeValueList;
}

GLBufferObject<GLuint>& Octree::getLeafIndexList()
{
    return ssboLeafIndexList;
}
GLBufferObject<FragStruct>& Octree::getFragList()
{
    return ssboFragList;
}
GLuint Octree::getInitialNodeCounterValue()
{
    return initialNodeCounterValue;
}
GLuint Octree::getInitialNodeValueCounterValue()
{
    return initialNodeValueCounterValue;
}
/*
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
*/
Octree::Octree()
{
}


Octree::~Octree()
{
}

void Octree::generateChildrenNodes(GLuint parentIndex, NodeStruct & parent, std::vector<NodeStruct>& list, GLuint & initialNode, GLuint & initialValue)
{
    parent.childPtr = initialNode; initialNode+=8;
    parent.valueIndex = initialValue; initialValue++;
    for (int i = 0; i < 8; i++) {
        NodeStruct n;
        n.parentPtr = parentIndex;
        list.push_back(n);
    }
}

std::vector<Octree::NodeStruct> Octree::generateOctree(int level)
{
    pregeneratedInitialOctree.clear();
    initialNodeCounterValue = 1; 
    initialNodeValueCounterValue = 0;
    std::vector<NodeStruct> octree{ {0, 0, 0, 0, 0} };
    std::vector<NodeStruct> pendingList;
    generateChildrenNodes(0, octree[0], pendingList, initialNodeCounterValue, initialNodeValueCounterValue);
    int startIndex = octree.size();
    int count = pendingList.size();
    for (auto & n : pendingList) {
        octree.push_back(n);
    }
    pendingList.clear();
    for (int i = 1; i < level; i++) {
        for (int j = startIndex; j < startIndex + count; j++) {
            generateChildrenNodes(j, octree[j], pendingList, initialNodeCounterValue, initialNodeValueCounterValue);
        }
        startIndex = octree.size();
        count = pendingList.size();
        for (auto & n : pendingList) {
            octree.push_back(n);
        }
        pendingList.clear();
    }
    pregeneratedInitialOctree = octree;
    return octree;
}
