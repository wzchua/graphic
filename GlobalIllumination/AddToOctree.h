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

    bool hasInitialized = false;
public:
    void initialize();
    void run(Octree & octree, GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<CounterBlock> & counterSet,
            GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList);
    AddToOctree();
    ~AddToOctree();
};

