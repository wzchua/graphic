#pragma once
#include "GLBufferObject.h"
#include "LogStruct.h"
#include "ShaderProgram.h"
#include "CounterBlock.h"
#include "Octree.h"
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
    void run(GLBufferObject<CounterBlock>& counterSet, Octree & octree);
    FilterOctree();
    ~FilterOctree();
};

