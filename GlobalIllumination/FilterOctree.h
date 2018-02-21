#pragma once
#include "NodeStruct.h"
#include "GLBufferObject.h"
#include "LogStruct.h"
class FilterOctree
{
private:

    bool hasInitialized = false;
public:
    void initialize();
    void run(GLBufferObject<NodeStruct> & nodeOctree, GLBufferObject<GLuint> & ssboLeafIndexList, GLuint textureColor, GLuint textureNormal);
    FilterOctree();
    ~FilterOctree();
};

