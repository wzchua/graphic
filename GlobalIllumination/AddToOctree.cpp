#include "AddToOctree.h"
#include "GlobalShaderComponents.h"


typedef GlobalShaderComponents GlobalCom;
void AddToOctree::initialize()
{
    if (hasInitialized) {
        return;
    }

    //computeWorkgroupShader.generateShader("./Shaders/AddToOctreeComputeWorkgroup.comp", ShaderProgram::COMPUTE);
    //computeWorkgroupShader.linkCompileValidate();
    //ssboIndirect.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(Indirect), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);

    std::stringstream computeShaderString;
    computeShaderString <<  GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    computeShaderString << GlobalCom::getGlobalVariablesUBOCode();
    computeShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    computeShaderString << fragStructShaderCodeString(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);
    computeShaderString << Octree::nodeStructShaderCodeString(GlobalCom::OCTREE_NODE_SSBO_BINDING) << Octree::nodeValueStructShaderCodeString(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING) << Octree::leafStructShaderCodeString(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);

    addToOctreeShader.generateShader(computeShaderString, "./Shaders/AddToOctree.comp", ShaderProgram::COMPUTE);
    addToOctreeShader.linkCompileValidate();
    ssboFragList2.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
}

void AddToOctree::run(Octree & octree, GLBufferObject<CounterBlock>& counterSet)
{
    addToOctreeShader.use();
    auto set = counterSet.getPtr();
    GLuint fragmentCount = set->fragmentCounter;
    set->noOfFragments = fragmentCount;
    set->fragmentCounter = 0;
    counterSet.unMapPtr();
    bool isOdd = true;

    auto & ssboFragList = octree.getFragList();

    //common bindings
    octree.getNodeList().bind(GlobalCom::OCTREE_NODE_SSBO_BINDING);
    octree.getLeafIndexList().bind(GlobalCom::OCTREE_LEAF_LIST_SSBO_BINDING);
    octree.getNodeValueList().bind(GlobalCom::OCTREE_NODE_VALUE_SSBO_BINDING);
    
    while (fragmentCount != 0) {

        if (isOdd) {
            ssboFragList2.bind(GlobalCom::FRAGMENT_LIST_SSBO_BINDING); //output
            ssboFragList.bind(GlobalCom::FRAGMENT_LIST_SECONDARY_SSBO_BINDING);  //input
        }
        else {
            ssboFragList.bind(GlobalCom::FRAGMENT_LIST_SSBO_BINDING);  //output
            ssboFragList2.bind(GlobalCom::FRAGMENT_LIST_SECONDARY_SSBO_BINDING); //input
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(std::ceil(fragmentCount / mWorkGroupSize.x), 1, 1);
        
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
