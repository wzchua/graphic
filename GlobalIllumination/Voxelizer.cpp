#include "Voxelizer.h"



Voxelizer::Voxelizer()
{
    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();

    octreeCompShader.generateShader("./Shaders/BuildOctree.comp", ShaderProgram::COMPUTE);
    octreeCompShader.linkCompileValidate();

    //atomic counter buffer
    glGenBuffers(1, &atomicNodeCountPtr);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicNodeCountPtr);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicBrickCountPtrX);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBrickCountPtrX);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, atomicBrickCountPtrX);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &atomicBrickCountPtrY);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBrickCountPtrY);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomicBrickCountPtrY);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    GLint size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << size << " bytes." << std::endl;
    std::cout << "wanting to set is " << sizeof(fragStruct) * count << " bytes." << std::endl;
    //fragmentlist buffer
    glGenBuffers(1, &ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //nodelist buffer
    glGenBuffers(1, &ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(nodeStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //synclist buffer
    glGenBuffers(1, &ssboSyncList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSyncList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(syncStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboSyncList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenTextures(1, &texture3DBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DLockList);
    glBindTexture(GL_TEXTURE_3D, texture3DLockList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
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
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    fragmentCount = ptr[0];
    ptr[0] = 1;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
    fragStruct* ptrf = (fragStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(fragStruct),
        GL_MAP_READ_BIT);
    for (int i = 0; i < fragmentCount; i++) {
        min.x = glm::min(ptrf[i].position[0], min.x);
        min.y = glm::min(ptrf[i].position[1], min.y);
        min.z = glm::min(ptrf[i].position[2], min.z);
        max.x = glm::max(ptrf[i].position[0], max.x);
        max.y = glm::max(ptrf[i].position[1], max.y);
        max.z = glm::max(ptrf[i].position[2], max.z);
    }
    fragStruct fr = ptrf[0];
    fr = ptrf[1000];
    fr = ptrf[2000];
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    currentShaderProgram = octreeCompShader.use();
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfFragments"), fragmentCount);

    glBindImageTexture(0, texture3DBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(1, texture3DLockList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    int workgroupX = std::ceil(fragmentCount / 512.0);
    glDispatchCompute(workgroupX, 1, 1);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        switch (err) {
        case GL_OUT_OF_MEMORY:
            std::cerr << "OpenGL error: Out of Memory" << std::endl;
            break;
            case GL_INVALID_VALUE:
            std::cerr << "OpenGL error: Invalid Value" << std::endl;
            break;
            case GL_INVALID_OPERATION:
                std::cerr << "OpenGL error: Invalid Value" << std::endl;
                break;
        default:
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    unsigned int nodeCount = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
    GLuint* ptr2 = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    nodeCount = ptr2[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);



}
