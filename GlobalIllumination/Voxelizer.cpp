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
    GLint size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << size << " bytes." << std::endl;
    std::cout << "frag " << size / sizeof(FragStruct) << " ." << std::endl;
    std::cout << "node " << size / sizeof(nodeStruct) << " ." << std::endl;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_UNIFORM_BLOCK_SIZE is " << size << " bytes." << std::endl;
    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE is " << work_grp_size[0] << ", " << work_grp_size[1] << ", " << work_grp_size[2] << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
    std::cout << "GL_MAX_3D_TEXTURE_SIZE  is " << size << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &size);
    std::cout << "GL_MAX_GEOMETRY_OUTPUT_VERTICES  is " << size << " ." << std::endl;

    octreeCompShader.generateShader("./Shaders/BuildOctreeSimple.comp", ShaderProgram::COMPUTE);
    octreeCompShader.linkCompileValidate();

    octreeAverageCompShader.generateShader("./Shaders/AverageOctreeBricks.comp", ShaderProgram::COMPUTE);
    octreeAverageCompShader.linkCompileValidate();

    octreeRenderShader.generateShader("./Shaders/VoxelOctreeRayCast.vert", ShaderProgram::VERTEX);
    octreeRenderShader.generateShader("./Shaders/VoxelOctreeRayCast.frag", ShaderProgram::FRAGMENT);
    octreeRenderShader.linkCompileValidate();

    glGenBuffers(1, &voxelMatrixUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelMatrixUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelMatrixBlock), &voxelMatrixData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &voxelLogUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelLogUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LimitsBlock), &voxelLogCountData, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    GLuint zero = 0;
    GLuint one = 1;

    atomicFragCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    atomicNodeCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &one, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    atomicNodeCounter.bind(1);
    atomicModelBrickCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &one, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    atomicModelBrickCounter.bind(2);
    atomicLeafNodeCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    atomicLeafNodeCounter.bind(3);
    //atomicLogCounter.initialize(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    mModuleToFragList.initialize();
    ssboCounterSet.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(CounterBlock), &mCounterBlock, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboFragmentList.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboFragmentList2.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboLogList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(LogStruct) * maxLogCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    ssboVoxelList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1024 * 1024, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    mModuleRenderToGrid.initialize();
    mModuleVoxelVisualizer.initialize();
    /*
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
    */
    glGenTextures(1, &texture3DColorGrid);
    glBindTexture(GL_TEXTURE_3D, texture3DColorGrid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_RGBA8, 512, 512, 512);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DNormalGrid);
    glBindTexture(GL_TEXTURE_3D, texture3DNormalGrid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_RGBA8, 512, 512, 512);
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
    voxelMatrixData.worldToVoxelMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    newMin = voxelMatrixData.worldToVoxelMat * glm::vec4(min, 1.0);
    newMax = voxelMatrixData.worldToVoxelMat * glm::vec4(max, 1.0);

    voxelMatrixData.viewProjMatrixXY = ortho * voxelViewMatriXY;
    voxelMatrixData.viewProjMatrixZY = ortho * voxelViewMatriZY;
    voxelMatrixData.viewProjMatrixXZ = ortho * voxelViewMatriXZ;

    glNamedBufferSubData(voxelMatrixUniformBuffer, 0, sizeof(VoxelMatrixBlock), &voxelMatrixData);
}

void Voxelizer::voxelizeFragmentList(Scene& scene)
{
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();
    int currentShaderProgram;// = voxelizeListShader.use();

    resetAllData();
    auto timeAfterReset = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    atomicFragCounter.bind(0);
    //mModuleToFragList.run(scene, ssboCounterSet, ssboFragmentList, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLogList);
    mModuleRenderToGrid.run(scene, ssboCounterSet, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLogList, texture3DColorGrid, texture3DNormalGrid, ssboVoxelList);
    auto timeAfterRender= std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();

    // get length of fragment list
    auto set = ssboCounterSet.getPtr();
    unsigned int voxelCount = set[0].fragmentCounter;
    unsigned int logCount = set[0].logCounter;
    set[0] = mZeroedCounterBlock;
    ssboCounterSet.unMapPtr();

    auto vPtr = ssboVoxelList.getPtr();
    std::vector<glm::vec4> voxels;
    for (int i = 0; i < voxelCount; i++) {
        voxels.push_back(vPtr[i]);
    }
    ssboVoxelList.unMapPtr();
    //auto ptrfrag = atomicFragCounter.getPtr();
   // auto t = ptrfrag[0];
    //ptrfrag[0] = 0;
    //atomicFragCounter.unMapPtr();
    std::vector<LogStruct> logs;
    //ShaderLogger::getLogs(ssboLogList, logCount, logs);

    //unsigned int fragmentCount = getAndResetCount(atomicFragCounter);

    //glGetNamedBufferSubData(atomicFragCounterTest, 0, sizeof(GLuint), &fragmentCount);
    auto timeAfterFragmentCountGet = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    std::cout << "time after reset: " << timeAfterReset << " ms. time after render: " << timeAfterRender << " ms. time after fragment count get: " << timeAfterFragmentCountGet << " ms" << std::endl;
    mModuleVoxelVisualizer.rayCastVoxels(scene.cam, voxelMatrixData.worldToVoxelMat, texture3DColorGrid);
    set = ssboCounterSet.getPtr();
    logCount = set[0].logCounter;
    set[0] = mZeroedCounterBlock;
    ssboCounterSet.unMapPtr();
    ShaderLogger::getLogs(ssboLogList, logCount, logs);
    auto timeAfterVoxelRayCast = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    std::cout << "time after voxel ray cast : " << timeAfterVoxelRayCast << " ms" << std::endl;

    mModuleRenderToGrid.resetData();
    glInvalidateTexImage(texture3DColorGrid, 0);
    glClearTexImage(texture3DColorGrid, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalGrid, 0);
    glClearTexImage(texture3DNormalGrid, 0, GL_RGBA, GL_FLOAT, NULL);
    /*
    glm::vec4 min = glm::vec4(0.0);
    glm::vec4 max = glm::vec4(0.0);
    std::vector<FragStruct> fListD;
    FragStruct* ptrf = ssboFragmentList.getPtr();
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
    */
    

    //std::vector<LogStruct> logsF;
    //getLogs(logsF);   

    /*
    currentShaderProgram = octreeCompShader.use();
    bool isOdd = true;

    glBindImageTexture(0, texture3DrgColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, texture3DbaColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, texture3DxyNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, texture3DzwNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    std::cout << "time before iter: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;


    while (fragmentCount != 0) {
        std::cout << fragmentCount << std::endl;
        glUniform1ui(glGetUniformLocation(currentShaderProgram, "noOfFragments"), fragmentCount);
        //swap bindings
        if (isOdd) {
            ssboFragmentList.bind(0);
            ssboFragmentList2.bind(1);
        }
        else {
            ssboFragmentList.bind(1);
            ssboFragmentList2.bind(0);
        }

        int workgroupX = std::ceil(fragmentCount / 512.0);

        glDispatchCompute(workgroupX, 1, 1);
        isOdd = !isOdd;

        fragmentCount = getAndResetCount(atomicFragCounter);
        std::cout << "time after iter: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;
    }
    std::cout << "time after octree: " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count() << "ms" << std::endl;


    int leafCount = getCount(atomicModelBrickCounter);
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
    */
    auto timeEnd = Clock::now();
    std::cout << "time end: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() << "ms" << std::endl;
}

void Voxelizer::resetAllData()
{
    GLuint zero = 0;
    //glNamedBufferSubData(atomicFragCounterTest, 0, sizeof(GLuint), &zero);
    glNamedBufferSubData(ssboCounterSet.getId(), 0, sizeof(CounterBlock), &mZeroedCounterBlock);
    getAndResetCount(atomicNodeCounter, 1);
    getAndResetCount(atomicModelBrickCounter, 1);
    getAndResetCount(atomicLeafNodeCounter);
    //getAndResetCount(atomicLogCounter);

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
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void Voxelizer::getLogs(std::vector<LogStruct>& logs, bool reset)
{
    int logCount = (reset) ? getAndResetCount(atomicLogCounter) : getCount(atomicLogCounter);
    LogStruct * ptr = ssboLogList.getPtr();

    for (int i = 0; i < logCount; i++) {
        logs.push_back(ptr[i]);
    }
}

int Voxelizer::getCount(GLBufferObject<GLuint>& counter)
{
    int count;
    GLuint* ptr = counter.getPtr();
    count = ptr[0];
    return count;
}

int Voxelizer::getAndResetCount(GLBufferObject<GLuint>& counter, int resetValue)
{
    int count;
    GLuint* ptr = counter.getPtr();
    count = ptr[0];
    ptr[0] = resetValue;
    return count;
}
