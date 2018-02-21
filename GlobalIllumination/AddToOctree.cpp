#include "AddToOctree.h"



void AddToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    //computeWorkgroupShader.generateShader("./Shaders/AddToOctreeComputeWorkgroup.comp", ShaderProgram::COMPUTE);
    //computeWorkgroupShader.linkCompileValidate();
    //ssboIndirect.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(Indirect), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);

    addToOctreeShader.generateShader("./Shaders/AddToOctree.comp", ShaderProgram::COMPUTE);
    addToOctreeShader.linkCompileValidate();
    ssboFragList2.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
}

void AddToOctree::run(GLBufferObject<NodeStruct>& ssboNodeList, GLBufferObject<FragStruct>& ssboFragList, GLBufferObject<CounterBlock>& counterSet, GLuint logUniformBlock, GLuint texture3DColor, GLuint texture3DNormal, GLBufferObject<LogStruct>& ssboLogList)
{
    auto set = counterSet.getPtr();
    GLuint fragmentCount = set->fragmentCounter;
    set->noOfFragments = fragmentCount;
    set->fragmentCounter = 0;
    counterSet.unMapPtr();
    bool isOdd = true;

    //common bindings
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);
    counterSet.bind(1);
    ssboNodeList.bind(2);
    ssboLogList.bind(7);
    glBindImageTexture(4, texture3DColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, texture3DNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    
    while (fragmentCount != 0) {
        addToOctreeShader.use();

        if (isOdd) {
            ssboFragList2.bind(0); //output
            ssboFragList.bind(3);  //input
        }
        else {
            ssboFragList.bind(0);  //output
            ssboFragList2.bind(3); //input
        }

        glDispatchCompute(std::ceil(fragmentCount / 512.0), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); 
        
        set = counterSet.getPtr();
        fragmentCount = set->fragmentCounter;
        set->noOfFragments = fragmentCount;
        set->fragmentCounter = 0;
        counterSet.unMapPtr();
        isOdd = !isOdd;
    }


}

AddToOctree::AddToOctree()
{
}


AddToOctree::~AddToOctree()
{
}
