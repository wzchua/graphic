#include "RenderToOctree.h"



typedef GlobalShaderComponents GlobalCom;
void RenderToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToGeomTripleOutput();
    vertShaderString << GlobalCom::getGlobalVariablesUBOCode() << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING);
    vertShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX_UBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    voxelizeOctreeShader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GlobalCom::getHeader() << GlobalCom::getGeomTripleInput() << GlobalCom::getGeomToFragTripleOutput();
    geomShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX_UBO_BINDING);
    voxelizeOctreeShader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getGlobalVariablesUBOCode();
    fragShaderString << fragStructShaderCodeString(GlobalCom::FRAGMENT_LIST_SSBO_BINDING) << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    fragShaderString << Octree::nodeStructShaderCodeString(GlobalCom::OCTREE_NODE_SSBO_BINDING) << Octree::nodeValueStructShaderCodeString(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING) << Octree::leafStructShaderCodeString(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);

    voxelizeOctreeShader.generateShader(fragShaderString, "./Shaders/VoxelizeOctree.frag", ShaderProgram::FRAGMENT);
    voxelizeOctreeShader.linkCompileValidate();
}

void RenderToOctree::run(Scene & inputScene, GLBufferObject<CounterBlock>& counterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, Octree & octree,
                            GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<LogStruct> & ssboLogList)
{
    GLuint currentShaderProgram = voxelizeOctreeShader.use();

    octree.getNodeList().bind(GlobalCom::OCTREE_NODE_SSBO_BINDING);
    octree.getNodeValueList().bind(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING);
    octree.getLeafIndexList().bind(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);

    //glBindImageTexture(4, octree.getTextureIds(Octree::TexType::COLOR), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    //glBindImageTexture(5, octree.getTextureIds(Octree::TexType::NORMAL), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
