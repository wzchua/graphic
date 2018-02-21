#include "RenderToOctree.h"



void RenderToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    voxelizeOctreeShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeOctreeShader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    voxelizeOctreeShader.generateShader("./Shaders/VoxelizeOctree.frag", ShaderProgram::FRAGMENT);
    voxelizeOctreeShader.linkCompileValidate();
}

void RenderToOctree::run(Scene & inputScene, GLBufferObject<CounterBlock>& counterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<GLuint> & ssboLeafIndexList,
                            GLBufferObject<NodeStruct>& ssboNodeList, GLuint texture3DColor, GLuint texture3DNormal, GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<LogStruct> & ssboLogList)
{
    GLuint currentShaderProgram = voxelizeOctreeShader.use();

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboFragList.bind(0);
    counterSet.bind(1);
    ssboNodeList.bind(2);
    ssboLeafIndexList.bind(3);
    ssboLogList.bind(7);

    glBindImageTexture(4, texture3DColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, texture3DNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    inputScene.render(currentShaderProgram);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

RenderToOctree::RenderToOctree()
{
}


RenderToOctree::~RenderToOctree()
{
}
