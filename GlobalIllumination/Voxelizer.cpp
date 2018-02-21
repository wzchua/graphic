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
    mModuleFilterOctree.initialize();
    mModuleRenderLightIntoOctree.initialize();

    glGenBuffers(1, &voxelMatrixUniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, voxelMatrixUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VoxelMatrixBlock), &voxelMatrixData, GL_STATIC_DRAW);
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
    
    /*
    ssboVoxelList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1024 * 1024 * 4, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    mModuleRenderToGrid.initialize();
    mModuleVoxelVisualizer.initialize();

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
*/
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
    std::vector<LogStruct> logs;
    using Clock = std::chrono::high_resolution_clock;
    auto timeStart = Clock::now();

    mModuleRenderToOctree.run(scene, ssboCounterSet, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLeafIndexList, ssboNodeList, texture3DColorList, texture3DNormalList, ssboFragmentList, ssboLogList);
    //mModuleRenderToGrid.run(scene, ssboCounterSet, voxelMatrixUniformBuffer, voxelLogUniformBuffer, ssboLogList, texture3DColorGrid, texture3DNormalGrid, ssboVoxelList);
    auto timeAfterRender= std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();

    //ShaderLogger::getLogs(ssboLogList, logCount, logs);
    mModuleAddToOctree.run(ssboNodeList, ssboFragmentList, ssboCounterSet, voxelLogUniformBuffer, texture3DColorList, texture3DNormalList, ssboLogList);

    auto timeAfterAddingToOctree = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - timeStart).count();
    std::cout << " ms. time after render: " << timeAfterRender << " ms. time after add to octree: " << timeAfterAddingToOctree << " ms" << std::endl;


    //filter octree

    // inject light

    // filter light

    // render cam RSM

    // cone trace octree and draw screen

    resetAllData();
    auto timeEnd = Clock::now();
    std::cout << "time end: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() << "ms" << std::endl;
}

void Voxelizer::resetAllData()
{
    /*
    mModuleRenderToGrid.resetData();
    glInvalidateTexImage(texture3DColorGrid, 0);
    glClearTexImage(texture3DColorGrid, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalGrid, 0);
    glClearTexImage(texture3DNormalGrid, 0, GL_RGBA, GL_FLOAT, NULL);*/


    glNamedBufferSubData(ssboCounterSet.getId(), 0, sizeof(CounterBlock), &mZeroedCounterBlock);

    ssboNodeList.clearData();    
    
    glInvalidateTexImage(texture3DColorList, 0);
    glClearTexImage(texture3DColorList, 0, GL_RGBA, GL_FLOAT, NULL);
    glInvalidateTexImage(texture3DNormalList, 0);
    glClearTexImage(texture3DNormalList, 0, GL_RGBA, GL_FLOAT, NULL);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
