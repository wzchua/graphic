#include "RenderLightIntoOctree.h"



void RenderLightIntoOctree::initialize()
{
    if (hasInitialized) {
        return;
    }
    injectLightOctreeShader.generateShader("./Shaders/RSM.vert", ShaderProgram::VERTEX);
    injectLightOctreeShader.generateShader("./Shaders/RSMOctree.frag", ShaderProgram::FRAGMENT);
    injectLightOctreeShader.linkCompileValidate();
}

void RenderLightIntoOctree::run(Scene & inputScene, Octree & octree, GLuint voxelizeMatrixBlock)
{
    glViewport(0, 0, 1024, 1024); // light render is done at 1024x1024
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    injectLightOctreeShader.use();
    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getLightMatrixBuffer()); // light as camera
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, inputScene.getLightBuffer());
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, voxelizeMatrixBlock);

    octree.getNodeList().bind(2);
    octree.getNodeValueList().bind(3);

    //glBindImageTexture(4, octree.getTextureIds(Octree::LIGHT_DIRECTION), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    //glBindImageTexture(5, octree.getTextureIds(Octree::LIGHT_ENERGY), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    inputScene.render(injectLightOctreeShader.getProgramId());


    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
    inputScene.render(injectLightOctreeShader.getProgramId());
    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    inputScene.render(injectLightOctreeShader.getProgramId());
    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    inputScene.render(injectLightOctreeShader.getProgramId());
    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
    inputScene.render(injectLightOctreeShader.getProgramId());
    inputScene.updateLightMatrixBufferForPointLight(0, glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
    inputScene.render(injectLightOctreeShader.getProgramId());


}

RenderLightIntoOctree::RenderLightIntoOctree()
{
}


RenderLightIntoOctree::~RenderLightIntoOctree()
{
}
