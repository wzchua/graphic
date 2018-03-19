#include "RenderToOctree.h"



void RenderToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream vertShaderString;
    vertShaderString << GenericShaderCodeString::vertHeader << GenericShaderCodeString::vertGeomOutput;
    vertShaderString << voxelizeBlockString(0) << GenericShaderCodeString::genericLimitsUniformBlock(7);
    vertShaderString << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);
    voxelizeOctreeShader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GenericShaderCodeString::geomHeader << GenericShaderCodeString::geomFragOutput;
    geomShaderString << voxelizeBlockString(0);
    voxelizeOctreeShader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GenericShaderCodeString::fragHeader;
    fragShaderString << GenericShaderCodeString::genericLimitsUniformBlock(7);
    fragShaderString << fragStructShaderCodeString(0) << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);
    fragShaderString << Octree::nodeStructShaderCodeString(2) << Octree::nodeValueStructShaderCodeString(3) << Octree::leafStructShaderCodeString(4);

    voxelizeOctreeShader.generateShader(fragShaderString, "./Shaders/VoxelizeOctree.frag", ShaderProgram::FRAGMENT);
    voxelizeOctreeShader.linkCompileValidate();
}

void RenderToOctree::run(Scene & inputScene, GLBufferObject<CounterBlock>& counterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, Octree & octree,
                            GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<LogStruct> & ssboLogList)
{
    GLuint currentShaderProgram = voxelizeOctreeShader.use();

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboFragList.bind(0);
    counterSet.bind(1);
    ssboLogList.bind(7);

    octree.getNodeList().bind(2);
    octree.getNodeValueList().bind(3);
    octree.getLeafIndexList().bind(4);

    //glBindImageTexture(4, octree.getTextureIds(Octree::TexType::COLOR), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    //glBindImageTexture(5, octree.getTextureIds(Octree::TexType::NORMAL), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

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
