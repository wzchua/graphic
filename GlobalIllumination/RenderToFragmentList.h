#pragma once
#include <glm/gtc/type_ptr.hpp>
#include "GLBufferObject.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "FragStruct.h"
#include "LogStruct.h"
#include "CounterBlock.h"
class RenderToFragmentList
{
private:
    struct logStruct {
        float position[4];
        float color[4];
    };
    ShaderProgram voxelizeListShader;

    bool hasInitialized = false;
public:
    int projectionAxis = 0;
    void initialize();
    void run(Scene& inputScene, GLBufferObject<CounterBlock> & counterSet, GLBufferObject<FragStruct> & ssboFragList, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList);
    RenderToFragmentList();
    ~RenderToFragmentList();
};

