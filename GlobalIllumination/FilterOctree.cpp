#include "FilterOctree.h"



void FilterOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    filterOctreeShader.generateShader("./Shaders/MIPmapOctree.comp", ShaderProgram::COMPUTE);
    filterOctreeShader.linkCompileValidate();
}

void FilterOctree::run(GLBufferObject<CounterBlock>& counterSet, GLBufferObject<NodeStruct>& nodeOctree, GLBufferObject<GLuint> & ssboLeafIndexList, GLuint textureColor, GLuint textureNormal, GLuint leafCount)
{
    GLuint currentShaderProgram = filterOctreeShader.use();

    counterSet.bind(1);
    nodeOctree.bind(2);
    ssboLeafIndexList.bind(3);
    glBindImageTexture(4, textureColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, textureNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    GLuint workGroupX = std::ceil(leafCount / 512.0);
    glDispatchCompute(workGroupX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //need to generate next layer of "leaves"

}

FilterOctree::FilterOctree()
{
}


FilterOctree::~FilterOctree()
{
}
