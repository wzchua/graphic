#pragma once
#include "ShaderProgram.h"
#include "NodeStruct.h"
#include "CounterBlock.h"
#include "GLBufferObject.h"
#include "FragStruct.h"
#include "LogStruct.h"

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
    void run(GLBufferObject<NodeStruct> & ssboNodeList, GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<CounterBlock> & counterSet, GLBufferObject<GLuint> & ssboLeafIndexList,
            GLuint logUniformBlock, GLuint texture3DColor, GLuint texture3DNormal, GLBufferObject<LogStruct> & ssboLogList);
    AddToOctree();
    ~AddToOctree();
};

