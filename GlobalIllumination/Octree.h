#pragma once
#include "glad\glad.h"

#include "GLBufferObject.h"
#include "NodeStruct.h"


class Octree
{
private:
    unsigned int fragCount = 1024 * 1024 * 2;
    unsigned int nodeCount = 1024 * 1024 * 2;
    int brickDim = 2;
    int texWdith = 512;
    int texHeight = 512;
    GLBufferObject<NodeStruct> ssboNodeList;
    GLBufferObject<GLuint> ssboLeafIndexList;
    GLuint texture3DColorList;
    GLuint texture3DNormalList;
    GLuint texture3DLightDirList;
    GLuint texture3DLightEnergyList;
    void initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
public:
    enum TexType { COLOR, NORMAL, LIGHT_DIRECTION, LIGHT_ENERGY };
    void initialize();
    void resetData();
    GLBufferObject<NodeStruct> & getNodeList();
    GLBufferObject<GLuint> & getLeafIndexList();
    GLuint getTextureIds(TexType type);
    Octree();
    ~Octree();
};
