#pragma once
#include "ShaderProgram.h"
#include "CounterBlock.h"
#include "GLBufferObject.h"
#include "FragStruct.h"
#include "LogStruct.h"
#include "Octree.h"

class AddToOctree
{
private:
    struct Indirect {
        GLuint x;
        GLuint y;
        GLuint z;
    };
    ShaderProgram computeWorkgroupShader;
    GLBufferObject<Indirect> ssboIndirect;


    ShaderProgram addToOctreeShader;
    GLBufferObject<FragStruct> ssboFragList2;
    GLuint fragCount = 1024 * 512;
    const glm::ivec3 mWorkGroupSize{ 512, 1, 1 };

    bool hasInitialized = false;
public:
    void initialize();
    void run(Octree & octree, GLBufferObject<CounterBlock> & counterSet);
    AddToOctree();
    ~AddToOctree();
};

