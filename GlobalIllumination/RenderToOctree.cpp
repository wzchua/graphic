#include "RenderToOctree.h"

typedef GlobalShaderComponents GlobalCom;
void RenderToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToGeomTripleOutput();
    vertShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX) << GlobalCom::getGlobalVariablesUBOCode();
    vertShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);

    shader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GlobalCom::getHeader() << GlobalCom::getGeomTripleInput() << GlobalCom::getGeomToFragTripleOutput();
    shader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getMaterialUBOCode() << GlobalCom::getGlobalVariablesUBOCode();
    fragShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    fragShaderString << fragStructShaderCodeString(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);
    fragShaderString << Octree::nodeStructShaderCodeString(GlobalCom::OCTREE_NODE_SSBO_BINDING) << Octree::nodeValueStructShaderCodeString(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING) << Octree::leafStructShaderCodeString(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);

    shader.generateShader(fragShaderString, "./Shaders/VoxelizeOctree.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
}

void RenderToOctree::run(Scene & inputScene, Octree & octree)
{
    GLuint currentShaderProgram = shader.use();

    octree.getFragList().bind(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);

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


    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    inputScene.render(currentShaderProgram);
}
