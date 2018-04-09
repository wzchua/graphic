#include "RenderLightIntoOctree.h"
#include "VoxelizeBlock.h"
#include "LogStruct.h"
#include "CounterBlock.h"


typedef GlobalShaderComponents GlobalCom;

void RenderLightIntoOctree::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    compShaderString << voxelizeBlockString(GlobalShaderComponents::VOXELIZATION_MATRIX_UBO_BINDING) << Scene::getLightUBOCode(GlobalShaderComponents::LIGHT_UBO_BINDING);
    compShaderString << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    compShaderString << Octree::nodeStructShaderCodeString(GlobalCom::OCTREE_NODE_SSBO_BINDING) << Octree::nodeValueStructShaderCodeString(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING) << Octree::leafStructShaderCodeString(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);
    shader.generateShader(compShaderString, "./Shaders/LightInjectionIntoOctree.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();;
}

void RenderLightIntoOctree::run(Scene & inputScene, Octree & octree, GLuint voxelizeMatrixBlock)
{

    shader.use();

    octree.getNodeList().bind(GlobalCom::OCTREE_NODE_SSBO_BINDING);
    octree.getNodeValueList().bind(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /*int numOfPointLight = inputScene.getTotalPointLights();

    for (int i = 0; i < numOfPointLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getPointLightBufferId(i));
        for (int j = 0; j < 6; j++) {
            RSM& rsm = inputScene.getPointLightRSM(i, j);
            auto res = rsm.getResolution();
            glBindTextureUnit(0, rsm.getVoxelPositionMap());
            glBindTextureUnit(1, rsm.getNormalMap());
            glDispatchCompute(res.x / mWorkGroupSize.x, res.y / mWorkGroupSize.y, 1);
        }
    }*/

    int numOfDirectionalLight = inputScene.getTotalDirectionalLights();
    for (int i = 0; i < numOfDirectionalLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getDirectionalLightBufferId(i));
        RSM& rsm = inputScene.getDirectionalLightRSM(i);
        auto res = rsm.getResolution();
        glBindTextureUnit(0, rsm.getVoxelPositionMap());
        glBindTextureUnit(1, rsm.getNormalMap());
        glDispatchCompute(res.x / mWorkGroupSize.x, res.y / mWorkGroupSize.y, 1);


    }
}