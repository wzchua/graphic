#include "FilterOctree.h"



void FilterOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    filterOctreeWorkgroupShader.generateShader("./Shaders/MIPmapOctreeComputeWorkgroup.comp", ShaderProgram::COMPUTE);
    filterOctreeWorkgroupShader.linkCompileValidate();
    ssboIndirect.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(Indirect), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    filterOctreeShader.generateShader("./Shaders/MIPmapOctree.comp", ShaderProgram::COMPUTE);
    filterOctreeShader.linkCompileValidate();
    secondaryLeafList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 1024 * 512, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
}

void FilterOctree::run(GLBufferObject<CounterBlock>& counterSet, GLBufferObject<NodeStruct>& nodeOctree, GLBufferObject<GLuint> & ssboLeafIndexList, GLuint textureColor, GLuint textureNormal, GLuint textureLightEnergy, GLuint textureLightDir)
{
    counterSet.bind(1);
    nodeOctree.bind(2);
    ssboIndirect.bind(6);

    glBindImageTexture(4, textureColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(5, textureNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glBindImageTexture(6, textureLightEnergy, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, textureLightDir, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, ssboIndirect.getId());

    bool isOdd = true;
    for (int i = 0; i < 8; i++) {
        filterOctreeWorkgroupShader.use();
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        if (isOdd) {
            ssboLeafIndexList.bind(3);
            secondaryLeafList.bind(4);
        }
        else {
            ssboLeafIndexList.bind(4);
            secondaryLeafList.bind(3);
        }

        filterOctreeShader.use();
        glDispatchComputeIndirect(0);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);

}

FilterOctree::FilterOctree()
{
}


FilterOctree::~FilterOctree()
{
}
