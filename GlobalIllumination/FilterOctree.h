#pragma once
#include "NodeStruct.h"
#include "GLBufferObject.h"
#include "LogStruct.h"
#include "ShaderProgram.h"
#include "CounterBlock.h"
class FilterOctree
{
private:
    struct Indirect {
        GLuint x;
        GLuint y;
        GLuint z;
    };

    ShaderProgram filterOctreeWorkgroupShader;
    GLBufferObject<Indirect> ssboIndirect;
    ShaderProgram filterOctreeShader;
    GLBufferObject<GLuint> secondaryLeafList;
    bool hasInitialized = false;
public:
    void initialize();
    void run(GLBufferObject<CounterBlock>& counterSet, GLBufferObject<NodeStruct> & nodeOctree, GLBufferObject<GLuint> & ssboLeafIndexList, GLuint textureColor, GLuint textureNormal, GLuint textureLightEnergy, GLuint textureLightDir);
    FilterOctree();
    ~FilterOctree();
};

