#include "Voxelizer.h"



Voxelizer::Voxelizer()
{
    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();

    octreeCompShader.generateShader("./Shaders/BuildOctreeSimple.comp", ShaderProgram::COMPUTE);
    octreeCompShader.linkCompileValidate();

    octreeAverageCompShader.generateShader("./Shaders/AverageOctreeBricks.comp", ShaderProgram::COMPUTE);
    octreeAverageCompShader.linkCompileValidate();
    
    //atomic counter buffer
    glGenBuffers(1, &atomicFragCountPtr);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicFragCountPtr);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicFragCountPtr);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicNodeCountPtr);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicNodeCountPtr);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_WRITE_BIT);
    ptr[0] = 1;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicModelBrickCounterPtr);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicModelBrickCounterPtr);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomicModelBrickCounterPtr);
    ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_WRITE_BIT);
    ptr[0] = 1;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicLeafNodeCountPtr);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicLeafNodeCountPtr);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, atomicLeafNodeCountPtr);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicLogCounter);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicLogCounter);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, atomicLogCounter);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    GLint size;
    std::cout << "max frag count " << count << "." << std::endl;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << size << " bytes." << std::endl;
    std::cout << "wanting to set is " << sizeof(fragStruct) * count << " bytes." << std::endl;
    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE is " << work_grp_size[0] << ", " << work_grp_size[1] << ", " << work_grp_size[2] << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
    std::cout << "GL_MAX_3D_TEXTURE_SIZE  is " << size << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_IMAGE_UNITS, &size);
    std::cout << "GL_MAX_IMAGE_UNITS  is " << size << " bytes." << std::endl;

    //fragmentlist buffer
    glGenBuffers(1, &ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //fragmentlist buffer
    glGenBuffers(1, &ssboFragmentList2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboFragmentList2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //nodelist buffer
    glGenBuffers(1, &ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(nodeStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //LeafNodeList buffer
    glGenBuffers(1, &ssboLeafNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLeafNodeList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboLeafNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //loglist buffer
    glGenBuffers(1, &ssboLogList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLogList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(logStruct) * maxLogCount, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboLogList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    
    glGenTextures(1, &texture3DrgColorBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DrgColorBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DbaColorBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DbaColorBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DxyNormalBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DxyNormalBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    glGenTextures(1, &texture3DzwNormalBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DzwNormalBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DCounterList);
    glBindTexture(GL_TEXTURE_3D, texture3DCounterList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DColorList);
    glBindTexture(GL_TEXTURE_3D, texture3DColorList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DNormalList);
    glBindTexture(GL_TEXTURE_3D, texture3DNormalList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);
}


Voxelizer::~Voxelizer()
{
}

void Voxelizer::initializeWithScene(glm::vec3 min, glm::vec3 max)
{
    //scale/translate scene into ortho box 512 x 512 x 512
    glm::vec3 length = (max - min);
    glm::vec3 translate = glm::vec3(0.0f) - (min + length / 2.0f);
    glm::vec3 scale = 512.0f / length;
    //fixed ratio scaling
    float smallestScale = glm::min(scale.x, glm::min(scale.y, scale.z));
    scale = glm::vec3(smallestScale);
    sceneMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    newMin = sceneMat * glm::vec4(min, 1.0);
    newMax = sceneMat * glm::vec4(max, 1.0);

    modelViewMat = voxelViewMatrix * sceneMat;
    modelViewProjMat = ortho * modelViewMat;

}

void Voxelizer::voxelizeFragmentList(Scene scene)
{
    int currentShaderProgram = voxelizeListShader.use();

    glViewport(0, 0, 512, 512); 
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(sceneMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    scene.render(currentShaderProgram);
    glm::vec4 min = glm::vec4(0.0);
    glm::vec4 max = glm::vec4(0.0);
    // get length of fragment list
    unsigned int fragmentCount = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicFragCountPtr);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    fragmentCount = ptr[0];
    ptr[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    std::vector<fragStruct> fListD;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
    fragStruct* ptrf = (fragStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(fragStruct),
        GL_MAP_READ_BIT);
    //980216
    for (int i = 0; i < fragmentCount; i++) {
        min.x = glm::min(ptrf[i].position[0], min.x);
        min.y = glm::min(ptrf[i].position[1], min.y);
        min.z = glm::min(ptrf[i].position[2], min.z);
        max.x = glm::max(ptrf[i].position[0], max.x);
        max.y = glm::max(ptrf[i].position[1], max.y);
        max.z = glm::max(ptrf[i].position[2], max.z);
        fListD.push_back(ptrf[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    unsigned int nodeCount = 0;
    std::vector<nodeStruct> nList;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
    GLuint* ptr2 = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    nodeCount = ptr2[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
    nodeStruct* ptrn = (nodeStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(nodeStruct),
        GL_MAP_READ_BIT);
    for (int i = 0; i < 64; i++) {
        nList.push_back(ptrn[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



    //fragmentCount = 1024;
    currentShaderProgram = octreeCompShader.use();
    bool isOdd = true;
    while (fragmentCount != 0) {
        glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfFragments"), fragmentCount);
        glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);

        glBindImageTexture(0, texture3DrgColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(1, texture3DbaColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(2, texture3DxyNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(3, texture3DzwNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

        //swap bindings
        if (isOdd) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboFragmentList);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList2);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboFragmentList2);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
        }
        else {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboFragmentList);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList2);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboFragmentList2);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
        }

        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        int workgroupX = std::ceil(fragmentCount / 512.0);

        glDispatchCompute(workgroupX, 1, 1);
        isOdd = !isOdd;

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicFragCountPtr);
        ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        fragmentCount = ptr[0];
        ptr[0] = 0;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        std::vector<logStruct> logs;
        getLogs(logs);

        nodeCount = 0;
        nList.clear();
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
        ptr2 = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
            GL_MAP_READ_BIT);
        nodeCount = ptr2[0];
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
        ptrn = (nodeStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(nodeStruct),
            GL_MAP_READ_BIT);
        for (int i = 0; i < nodeCount; i++) {
            nList.push_back(ptrn[i]);
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    int leafCount = getCount(atomicModelBrickCounterPtr);
    std::vector<logStruct> logs;
    getLogs(logs, true);
    currentShaderProgram = octreeAverageCompShader.use();
    //averge brick value
    glBindImageTexture(4, texture3DColorList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(5, texture3DNormalList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfBricks"), leafCount);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    int workgroupX = std::ceil(leafCount / 512.0);

    glDispatchCompute(workgroupX, 1, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    std::vector<logStruct> logs2;
    getLogs(logs2);

    std::cout << "averaged" << std::endl;

}

void Voxelizer::getLogs(std::vector<logStruct>& logs, bool reset)
{
    int logCount = (reset) ? getAndResetCount(atomicLogCounter) : getCount(atomicLogCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLogList);
    logStruct * ptr = (logStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(logStruct),
        GL_MAP_READ_BIT);

    for (int i = 0; i < logCount; i++) {
        logs.push_back(ptr[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

int Voxelizer::getCount(GLuint counterId)
{
    int count;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counterId);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT);
    count = ptr[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    return count;
}

int Voxelizer::getAndResetCount(GLuint counterId, int resetValue)
{
    int count;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counterId);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT);
    count = ptr[0];
    ptr[0] = resetValue;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    return count;
}
