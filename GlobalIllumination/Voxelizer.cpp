#include "Voxelizer.h"

void CheckGLError()
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
        std::cout << "GL_" << error.c_str() << std::endl;
        err = glGetError();
    }

    return;
}

Voxelizer::Voxelizer()
{
    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();

    octreeCompShader.generateShader("./Shaders/BuildOctreeSimple.comp", ShaderProgram::COMPUTE);
    octreeCompShader.linkCompileValidate();

    octreeAverageCompShader.generateShader("./Shaders/AverageOctreeBricks.comp", ShaderProgram::COMPUTE);
    octreeAverageCompShader.linkCompileValidate();

    octreeRenderShader.generateShader("./Shaders/VoxelOctreeRayCast.vert", ShaderProgram::VERTEX);
    octreeRenderShader.generateShader("./Shaders/VoxelOctreeRayCast.frag", ShaderProgram::FRAGMENT);
    octreeRenderShader.linkCompileValidate();
    
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
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << size << " bytes." << std::endl;
    std::cout << "frag " << size/sizeof(fragStruct) << " ." << std::endl;
    std::cout << "node " << size/sizeof(nodeStruct) << " ." << std::endl;
    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE is " << work_grp_size[0] << ", " << work_grp_size[1] << ", " << work_grp_size[2] << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
    std::cout << "GL_MAX_3D_TEXTURE_SIZE  is " << size << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &size);
    std::cout << "GL_MAX_GEOMETRY_OUTPUT_VERTICES  is " << size << " ." << std::endl;

    //fragmentlist buffer
    glGenBuffers(1, &ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * fragCount, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboFragmentList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //fragmentlist buffer
    glGenBuffers(1, &ssboFragmentList2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboFragmentList2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * fragCount, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboFragmentList2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //nodelist buffer
    glGenBuffers(1, &ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(nodeStruct) * nodeCount, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    //LeafNodeList buffer
    glGenBuffers(1, &ssboLeafNodeList);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLeafNodeList);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * nodeCount, NULL, GL_DYNAMIC_DRAW);
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

    //cube vao
    std::vector<glm::vec3> quadVertices;
    quadVertices.push_back(glm::vec3(1.0, 1.0, 0.0));
    quadVertices.push_back(glm::vec3(1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, 1.0, 0.0));

    std::vector<GLuint> quadindices;
    quadindices.push_back(0);
    quadindices.push_back(3);
    quadindices.push_back(1);
    quadindices.push_back(2);
    quadindices.push_back(1);
    quadindices.push_back(3);

    int vertexSize = sizeof(glm::vec3);
    glGenVertexArrays(1, &quadVAOId);
    glBindVertexArray(quadVAOId);

    glGenBuffers(1, &quadVBOId);
    glGenBuffers(1, &quadEBOId);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBOId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadindices.size() * sizeof(GLuint), quadindices.data(), GL_STATIC_DRAW); //GL_DYNAMIC_DRAW

    glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * vertexSize, quadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}


Voxelizer::~Voxelizer()
{
}

void Voxelizer::initializeWithScene(glm::vec3 min, glm::vec3 max)
{
    //scale/translate scene into ortho box 512 x 512 x 512
    glm::vec3 length = (max - min);
    glm::vec3 translate = glm::vec3(0.0f) - min;
    glm::vec3 scale = 512.0f / length;
    //fixed ratio scaling
    float smallestScale = glm::min(scale.x, glm::min(scale.y, scale.z));
    scale = glm::vec3(smallestScale);
    worldToVoxelMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    newMin = worldToVoxelMat * glm::vec4(min, 1.0);
    newMax = worldToVoxelMat * glm::vec4(max, 1.0);

    viewProjMatrixXY = ortho * voxelViewMatriXY;
    viewProjMatrixZY = ortho * voxelViewMatriZY;
    viewProjMatrixXZ = ortho * voxelViewMatriXZ;
}

void Voxelizer::voxelizeFragmentList(Scene& scene)
{
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();
    int currentShaderProgram = voxelizeListShader.use();
    resetAllData();
    std::cout << "time after reset: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(worldToVoxelMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixXY"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixXY));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixZY"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixZY));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ViewProjMatrixXZ"), 1, GL_FALSE, glm::value_ptr(viewProjMatrixXZ));
    glUniform1i(glGetUniformLocation(currentShaderProgram, "projectionAxis"), projectionAxis);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    scene.render(currentShaderProgram);
    std::cout << "time after render: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;

    // get length of fragment list
    unsigned int fragmentCount = getAndResetCount(atomicFragCountPtr);
    /*
    glm::vec4 min = glm::vec4(0.0);
    glm::vec4 max = glm::vec4(0.0);
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
    */
    

    //std::vector<logStruct> logsF;
    //getLogs(logsF);   


    currentShaderProgram = octreeCompShader.use();
    bool isOdd = true;

    glBindImageTexture(0, texture3DrgColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, texture3DbaColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, texture3DxyNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, texture3DzwNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);
    std::cout << "time before iter: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;


    while (fragmentCount != 0) {
        std::cout << fragmentCount << std::endl;
        glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfFragments"), fragmentCount);
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

        int workgroupX = std::ceil(fragmentCount / 512.0);

        glDispatchCompute(workgroupX, 1, 1);
        isOdd = !isOdd;

        fragmentCount = getAndResetCount(atomicFragCountPtr);
        std::cout << "time after iter: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;


        //std::vector<logStruct> logs;
        //getLogs(logs);

        /*
        unsigned int nodeCount = 0;
        std::vector<nodeStruct> nList;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicNodeCountPtr);
        GLuint* ptr2 = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        nodeCount = ptr2[0];
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        std::cout << "nodeCount: " << nodeCount << std::endl;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboNodeList);
        nodeStruct* ptrn = (nodeStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(nodeStruct),
            GL_MAP_READ_BIT);
        for (int i = 0; i < nodeCount; i++) {
            nList.push_back(ptrn[i]);
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        */

    }
    std::cout << "time after octree: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;


    int leafCount = getCount(atomicModelBrickCounterPtr);
    std::cout << "Leaves: " << leafCount << std::endl;
    //std::vector<logStruct> logs;
    //getLogs(logs, true);
    currentShaderProgram = octreeAverageCompShader.use();
    //averge brick value
    glBindImageTexture(4, texture3DColorList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(5, texture3DNormalList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfBricks"), leafCount);
    glUniform1ui(glGetUniformLocation(currentShaderProgram, "maxNoOfLogs"), maxLogCount);

    int workgroupX = std::ceil(leafCount / 512.0);
    glDispatchCompute(workgroupX, 1, 1);
    //std::vector<logStruct> logs2;
    //getLogs(logs2, true);
    std::cout << "time after octree avg: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;
    
    //render octree
    currentShaderProgram = octreeRenderShader.use();

    glm::mat4 inverseViewMatrix = glm::inverse(camVoxel.getViewMatrix());
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "WorldToVoxelMat"), 1, GL_FALSE, glm::value_ptr(worldToVoxelMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "InverseViewMatrix"), 1, GL_FALSE, glm::value_ptr(inverseViewMatrix));
    glm::vec3 position = scene.cam.getPosition();
    glm::vec3 forward = scene.cam.getForward();
    glm::vec3 up = scene.cam.getUp();
    glUniform3fv(glGetUniformLocation(currentShaderProgram, "camPosition"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(currentShaderProgram, "camForward"), 1, glm::value_ptr(forward));
    glUniform3fv(glGetUniformLocation(currentShaderProgram, "camUp"), 1, glm::value_ptr(up));
    int width = 800;
    int height = 600;
    glUniform1i(glGetUniformLocation(currentShaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(currentShaderProgram, "height"), height);

    glViewport(0, 0, width, height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBindVertexArray(quadVAOId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    //logs2.clear();
    //getLogs(logs2, true);

    auto timeEnd = Clock::now();
    std::cout << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() << "ms" << std::endl;
}

void Voxelizer::resetAllData()
{
    //getAndResetCount(atomicFragCountPtr);
    glInvalidateBufferData(atomicFragCountPtr);
    glClearNamedBufferData(atomicFragCountPtr, GL_RG32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    getAndResetCount(atomicNodeCountPtr, 1);
    getAndResetCount(atomicModelBrickCounterPtr, 1);
    //getAndResetCount(atomicLeafNodeCountPtr);
    glInvalidateBufferData(atomicLeafNodeCountPtr);
    glClearNamedBufferData(atomicLeafNodeCountPtr, GL_RG32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    //getAndResetCount(atomicLogCounter);
    glInvalidateBufferData(atomicLeafNodeCountPtr);
    glClearNamedBufferData(atomicLeafNodeCountPtr, GL_RG32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

    glInvalidateBufferData(ssboNodeList);
    glClearNamedBufferData(ssboNodeList, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
    glInvalidateBufferData(ssboLeafNodeList);
    glClearNamedBufferData(ssboLeafNodeList, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
    
    glInvalidateTexImage(texture3DrgColorBrickList, 0);
    glClearTexImage(texture3DrgColorBrickList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glInvalidateTexImage(texture3DbaColorBrickList, 0);
    glClearTexImage(texture3DbaColorBrickList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glInvalidateTexImage(texture3DxyNormalBrickList, 0);
    glClearTexImage(texture3DxyNormalBrickList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glInvalidateTexImage(texture3DzwNormalBrickList, 0);
    glClearTexImage(texture3DzwNormalBrickList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

    glInvalidateTexImage(texture3DCounterList, 0);
    glClearTexImage(texture3DCounterList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glInvalidateTexImage(texture3DColorList, 0);
    glClearTexImage(texture3DColorList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalList, 0);
    glClearTexImage(texture3DNormalList, 0, GL_RGBA, GL_FLOAT, NULL);
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
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    count = ptr[0];
    ptr[0] = resetValue;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    return count;
}
