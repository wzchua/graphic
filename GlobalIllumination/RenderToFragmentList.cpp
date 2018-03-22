#include "RenderToFragmentList.h"


void CheckGLError3()
{
    GLenum err(glGetError());

    while (err != GL_NO_ERROR)
    {
        std::string error;
        switch (err)
        {
        case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        std::cout << "R2FL: GL_" << error.c_str() << std::endl;
        err = glGetError();
    }

    return;
}

void RenderToFragmentList::initialize()
{
    if (hasInitialized) {
        return;
    }

    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();
}

void RenderToFragmentList::run(Scene& inputScene, GLBufferObject<CounterBlock> & counterSet,
                                GLBufferObject<FragStruct> & ssboFragList, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList)
{
    GLuint currentShaderProgram = voxelizeListShader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboFragList.bind(0);
    counterSet.bind(1);
    ssboLogList.bind(7);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    inputScene.render(currentShaderProgram);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    /*
   auto syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
   GLenum waitReturn = GL_UNSIGNALED;
   while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
   {
       waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 20);
   }
   glDeleteSync(syncObj);*/
   
}

RenderToFragmentList::RenderToFragmentList()
{
}


RenderToFragmentList::~RenderToFragmentList()
{
}
