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
    std::cout << "node " << size / sizeof(NodeStruct) << " ." << std::endl;
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

    mModuleRenderToOctree.initialize();
    mModuleAddToOctree.initialize();
    //mModuleRenderLightIntoOctree.initialize();
    //mModuleFilterOctree.initialize();
    //mModuleRenderVCT.initialize();
    mModuleVoxelVisualizer.initialize();

    glGenBuffers(1, &voxelMatrixUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelMatrixUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelizeBlock), &voxelMatrixData, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &voxelLogUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelLogUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LimitsBlock), &voxelLogCountData, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    ssboCounterSet.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(CounterBlock), &mCounterBlock, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboFragmentList.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboLogList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(LogStruct) * maxLogCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    ssboNodeList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(NodeStruct) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ssboLeafIndexList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * nodeCount, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
        
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

    glGenTextures(1, &texture3DLightDirList);
    glBindTexture(GL_TEXTURE_3D, texture3DLightDirList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_RGBA8, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DLightEnergyList);
    glBindTexture(GL_TEXTURE_3D, texture3DLightEnergyList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 2, GL_R32UI, texWdith * brickDim, texHeight * brickDim, brickDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    if (!isOctree) {
        ssboVoxelList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1024 * 1024 * 4, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        mModuleRenderToGrid.initialize();

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
    }
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

    glNamedBufferSubData(voxelMatrixUniformBuffer, 0, sizeof(VoxelizeBlock), &voxelMatrixData);
}

void Voxelizer::render(Scene& scene)
{
    std::vector<LogStruct> logs;
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();

    if (isOctree) {
        mModuleRenderToOctree.run(scene, ssboCounterSet, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLeafIndexList, ssboNodeList, texture3DColorList, texture3DNormalList, ssboFragmentList, ssboLogList);
        auto syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        GLenum waitReturn = GL_UNSIGNALED;
        while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
        {
            waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 2);
        }
        glDeleteSync(syncObj);
        auto timeAfterRender = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
        auto cPtr = ssboCounterSet.getPtr();
        auto c2 = *cPtr;
        ssboCounterSet.unMapPtr();
        auto node = ssboNodeList.getPtr();
        std::vector<NodeStruct> nodeList;
        for (int i = 0; i < c2.nodeCounter; i++) {
            nodeList.push_back(node[i]);
        }
        ssboNodeList.unMapPtr();
        //mModuleAddToOctree.run(ssboNodeList, ssboFragmentList, ssboCounterSet, ssboLeafIndexList, voxelLogUniformBuffer, texture3DColorList, texture3DNormalList, ssboLogList);
        ssboNodeList.bind(2);
        mModuleVoxelVisualizer.rayCastVoxels(scene.cam, voxelMatrixData.worldToVoxelMat, ssboCounterSet, voxelLogUniformBuffer, texture3DColorList, VoxelVisualizer::OCTREE, ssboLogList);
        auto timeAfterAddingToOctree = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
        std::cout << " ms. time after render: " << timeAfterRender << " ms. time after add to octree: " << timeAfterAddingToOctree << " ms" << std::endl;

        // inject light
        //mModuleRenderLightIntoOctree.run(scene, ssboNodeList, texture3DLightEnergyList, texture3DLightDirList, voxelMatrixUniformBuffer);

        // filter octree geometry / light
        //mModuleFilterOctree.run(ssboCounterSet, ssboNodeList, ssboLeafIndexList, texture3DColorList, texture3DNormalGrid, texture3DLightEnergyList, texture3DLightDirList);

        // render cam RSM and draw shading using VCT
        //mModuleRenderVCT.run(scene, ssboCounterSet, ssboNodeList, texture3DColorList, texture3DNormalGrid, texture3DLightEnergyList, texture3DLightDirList);
    }
    else {
        mModuleRenderToGrid.run(scene, ssboCounterSet, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLogList, texture3DColorGrid, texture3DNormalGrid, ssboVoxelList);
        mModuleVoxelVisualizer.rayCastVoxels(scene.cam, voxelMatrixData.worldToVoxelMat, ssboCounterSet, voxelLogUniformBuffer, texture3DColorGrid, VoxelVisualizer::GRID, ssboLogList);
    }


    //ShaderLogger::getLogs(ssboLogList, logCount, logs);


    resetAllData();
    auto timeEnd = Clock::now();
    std::cout << "time end: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() << "ms" << std::endl;
}

void Voxelizer::resetAllData()
{
    CheckGLError();
    if (isOctree) {
        ssboNodeList.clearData();
        auto node = ssboNodeList.getPtr();
        std::vector<NodeStruct> nodeList;
        for (int i = 0; i < 50; i++) {
            nodeList.push_back(node[i]);
        }
        ssboNodeList.unMapPtr();

        glInvalidateTexImage(texture3DColorList, 0);
        glClearTexImage(texture3DColorList, 0, GL_RGBA, GL_FLOAT, NULL);
        glInvalidateTexImage(texture3DNormalList, 0);
        glClearTexImage(texture3DNormalList, 0, GL_RGBA, GL_FLOAT, NULL);
        glInvalidateTexImage(texture3DLightEnergyList, 0);
        glClearTexImage(texture3DLightEnergyList, 0, GL_RGBA, GL_FLOAT, NULL);
        glInvalidateTexImage(texture3DLightDirList, 0);
        glClearTexImage(texture3DLightDirList, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    else {
        mModuleRenderToGrid.resetData();
        glInvalidateTexImage(texture3DColorGrid, 0);
        glClearTexImage(texture3DColorGrid, 0, GL_RGBA, GL_FLOAT, NULL);
        glInvalidateTexImage(texture3DNormalGrid, 0);
        glClearTexImage(texture3DNormalGrid, 0, GL_RGBA, GL_FLOAT, NULL);
    }

    auto cPtr = ssboCounterSet.getPtr();
    cPtr[0] = mZeroedCounterBlock;
    ssboCounterSet.unMapPtr();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
}
