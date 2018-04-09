#include "RenderToFragmentList.h"
#include "GlobalShaderComponents.h"
#include "VoxelizeBlock.h"


typedef GlobalShaderComponents GlobalCom;

void RenderToFragmentList::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToGeomTripleOutput();
    vertShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX) << GlobalCom::getGlobalVariablesUBOCode();
    vertShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);

    voxelizeListShader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GlobalCom::getHeader() << GlobalCom::getGeomTripleInput() << GlobalCom::getGeomToFragTripleOutput();
    voxelizeListShader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getMaterialUBOCode() << GlobalCom::getGlobalVariablesUBOCode();
    fragShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    fragShaderString << fragStructShaderCodeString(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);
    voxelizeListShader.generateShader(fragShaderString, "./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();
}

void RenderToFragmentList::run(Scene& inputScene, GLBufferObject<FragStruct> & ssboFragList)
{
    GLuint currentShaderProgram = voxelizeListShader.use();
    ssboFragList.bind(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    inputScene.render(currentShaderProgram);   
}

RenderToFragmentList::RenderToFragmentList()
{
}


RenderToFragmentList::~RenderToFragmentList()
{
}
