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
    std::cout << "node " << size / sizeof(Octree::NodeStruct) << " ." << std::endl;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_UNIFORM_BLOCK_SIZE is " << size << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &size);
    std::cout << "GL_MAX_TEXTURE_IMAGE_UNITS is " << size << " ." << std::endl; 
    int work_grp_size[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
    std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE is " << work_grp_size[0] << ", " << work_grp_size[1] << ", " << work_grp_size[2] << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
    std::cout << "GL_MAX_3D_TEXTURE_SIZE  is " << size << " bytes." << std::endl;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &size);
    std::cout << "GL_MAX_GEOMETRY_OUTPUT_VERTICES  is " << size << " ." << std::endl;

    GlobalShaderComponents::initializeUBO(voxelMatrixUBOId, sizeof(VoxelizeBlock), &voxelMatrixData, GL_STATIC_DRAW);
    GlobalShaderComponents::initializeUBO(globalVariablesUBOId, sizeof(GlobalsBlock), &globalVariablesData, GL_STATIC_DRAW);
    ssboCounterSet.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(CounterBlock), &mCounterBlock, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0);
    ShaderLogger::initilizeLogBuffer(ssboLogList, maxLogCount);

    mModuleVoxelVisualizer.initialize();
    mModuleGBufferGen.initialize();
    mModuleLightRenderer.initialize();
    mModuleComputeShadows.initialize();
    mModuleFrameMuxer.initialize();
    switch(mType) {
    case OCTREE:
        mOctree.initialize();

        mModuleRenderToOctree.initialize();
        mModuleAddToOctree.initialize();
        //mModuleRenderLightIntoOctree.initialize();
        //mModuleFilterOctree.initialize();
        //mModuleRenderVCT.initialize();
        ssboFragmentList.initialize(GL_SHADER_STORAGE_BUFFER, fragCount * sizeof(FragStruct), NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, 0); 

        break;
    case CAS_GRID:
        mCascadedGrid.initializeGrids(3);
        mModuleRenderToCasGrid.initialize();
        mModuleRenderLightIntoCasGrid.initialize();
        mModuleRenderVoxelConeTraceCasGrid.initialize();
        break;
    case GRID:
        ssboVoxelList.initialize(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1024 * 1024 * 4, NULL, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        mModuleRenderToGrid.initialize();

        initialize3DTextures(texture3DColorGrid, 9, GL_RGBA8, 512, 512, 512);
        initialize3DTextures(texture3DNormalGrid, 9, GL_RGBA8, 512, 512, 512);
        break;

    default:
        throw new std::exception("Invalid enum mType");
    }

    //bindings
    GlobalShaderComponents::bindUBO(voxelMatrixUBOId, GlobalShaderComponents::UBOType::VOXELIZATION_MATRIX);
    GlobalShaderComponents::bindUBO(globalVariablesUBOId, GlobalShaderComponents::UBOType::GLOBAL_VARIABLES);

    ssboCounterSet.bind(GlobalShaderComponents::COUNTER_SSBO_BINDING);
    ssboLogList.bind(GlobalShaderComponents::LOG_SSBO_BINDING);

}


Voxelizer::~Voxelizer()
{
}

void Voxelizer::initializeWithScene(glm::vec3 min, glm::vec3 max)
{
    //scale/translate scene into ortho box 512 x 512 x 512
    glm::vec3 length = (max - min);
    glm::vec3 translate = glm::vec3(0.0f) - min;
    glm::vec3 scale = 511.0f / length;
    //fixed ratio scaling
    float smallestScale = glm::min(scale.x, glm::min(scale.y, scale.z));
    scale = glm::vec3(smallestScale);
    voxelMatrixData.worldToVoxelMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    newMin = voxelMatrixData.worldToVoxelMat * glm::vec4(min, 1.0);
    newMax = voxelMatrixData.worldToVoxelMat * glm::vec4(max, 1.0);

    voxelMatrixData.viewProjMatrixXY = ortho * voxelViewMatriXY;
    //voxelMatrixData.viewProjMatrixZY = ortho * voxelViewMatriZY;
    //voxelMatrixData.viewProjMatrixXZ = ortho * voxelViewMatriXZ;

    glNamedBufferSubData(voxelMatrixUBOId, 0, sizeof(VoxelizeBlock), &voxelMatrixData);
}

void Voxelizer::render(Scene& scene)
{
    OpenGLTimer timer;
    timer.setTimestamp();
    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalCom::CAMERA_MATRIX_UBO_BINDING, scene.getMatrixBuffer());
    mModuleLightRenderer.run(scene); timer.setTimestamp();
    mModuleGBufferGen.run(scene, mGBuffer); timer.setTimestamp();
    //mGBuffer.dumpBuffersAsImages();
    //std::vector<LogStruct> logs;

    switch (mType) {
    case OCTREE:
    {
        mModuleRenderToOctree.run(scene, ssboCounterSet, voxelMatrixUBOId, globalVariablesUBOId, mOctree, ssboFragmentList, ssboLogList); timer.setTimestamp();
        auto cPtr = ssboCounterSet.getPtr();
        auto c2 = *cPtr;
        ssboCounterSet.unMapPtr();
        auto node = mOctree.getNodeList().getPtr();
        std::vector<Octree::NodeStruct> nodeList;
        for (int i = 0; i < c2.nodeCounter; i++) {
            nodeList.push_back(node[i]);
        }
        mOctree.getNodeList().unMapPtr();
        //mModuleAddToOctree.run(ssboNodeList, ssboFragmentList, ssboCounterSet, ssboLeafIndexList, voxelLogUniformBuffer, texture3DColorList, texture3DNormalList, ssboLogList); timer.setTimestamp();
        mModuleVoxelVisualizer.rayCastVoxels(scene.cam, voxelMatrixData.worldToVoxelMat, mOctree, currentNumMode);

        // inject light
        //mModuleRenderLightIntoOctree.run(scene, ssboNodeList, texture3DLightEnergyList, texture3DLightDirList, voxelMatrixUniformBuffer); timer.setTimestamp();

        // filter octree geometry / light
        //mModuleFilterOctree.run(ssboCounterSet, ssboNodeList, ssboLeafIndexList, texture3DColorList, texture3DNormalGrid, texture3DLightEnergyList, texture3DLightDirList); timer.setTimestamp();

        // render cam RSM and draw shading using VCT
        //mModuleRenderVCT.run(scene, ssboCounterSet, ssboNodeList, texture3DColorList, texture3DNormalGrid, texture3DLightEnergyList, texture3DLightDirList); timer.setTimestamp();
    }
    break;
    case CAS_GRID:
    {
        mModuleRenderToCasGrid.run(scene, voxelMatrixData.worldToVoxelMat, mCascadedGrid); timer.setTimestamp();
        mModuleRenderLightIntoCasGrid.run(scene, voxelMatrixUBOId, mCascadedGrid); timer.setTimestamp();
        mCascadedGrid.filter(); timer.setTimestamp();
        mModuleRenderVoxelConeTraceCasGrid.run(scene, ssboCounterSet, mCascadedGrid, ssboLogList, mGBuffer); timer.setTimestamp();
    }
    break;
    case GRID:
    {
        mModuleRenderToGrid.run(scene, ssboCounterSet, voxelMatrixUBOId, globalVariablesUBOId, ssboLogList, texture3DColorGrid, texture3DNormalGrid, ssboVoxelList);
        mModuleVoxelVisualizer.rayCastVoxelsGrid(scene.cam, voxelMatrixData.worldToVoxelMat, texture3DColorGrid, currentNumMode);
    }
    break;
    default:
        break;
    }


    //ShaderLogger::getLogs(ssboLogList, logCount, logs);
    if (currentNumMode == 0) {
        //render shadows using rsm and gBUffer
        mModuleComputeShadows.run(scene, mGBuffer); timer.setTimestamp(); 
        /*auto c = ssboCounterSet.getPtr();
        int logCount = c->logCounter;
        c->logCounter = 0;
        ssboCounterSet.unMapPtr();
        std::vector<LogStruct> logs;
        ShaderLogger::getLogs(ssboLogList, logCount, logs);
        std::cout << "h\n";*/
        //add indirect illumination
        mModuleFrameMuxer.run(mGBuffer); timer.setTimestamp();
        //bilts to framebuffer
        mGBuffer.blitFinalToScreen(); timer.setTimestamp();

        if (toDumpCurrentGBuffer) {
            toDumpCurrentGBuffer = false;
            mGBuffer.dumpBuffersAsImages();
            scene.getDirectionalLightRSM(0).dumpAsImage("0");
        }
    }
    else {
        //if casgrid
        switch (mType) {
        case OCTREE:
            break;
        case CAS_GRID:
            mModuleVoxelVisualizer.rayCastVoxels(scene.cam, voxelMatrixData.worldToVoxelMat, mCascadedGrid, currentNumMode, gridDefinition); timer.setTimestamp();
            break;
        }
    }

    resetAllData(); timer.setTimestamp();
    auto times = timer.getElapsedTime();
    for (auto t : times) {
        std::cout << std::to_string(t / 1000000.0) << "ms ";
    }
    std::cout << "\n\n";
}

void Voxelizer::resetAllData()
{
    switch (mType) {
    case OCTREE:
    {
        mOctree.resetData();
    }
    break;
    case CAS_GRID:
    {
        mCascadedGrid.resetData();
    }
    break;
    case GRID:
    {
        mModuleRenderToGrid.resetData();
        glInvalidateTexImage(texture3DColorGrid, 0);
        glClearTexImage(texture3DColorGrid, 0, GL_RGBA, GL_FLOAT, NULL);
        glInvalidateTexImage(texture3DNormalGrid, 0);
        glClearTexImage(texture3DNormalGrid, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    break;
    }

    auto cPtr = ssboCounterSet.getPtr();
    cPtr[0] = mZeroedCounterBlock;
    ssboCounterSet.unMapPtr();
}

void Voxelizer::onNumberPressed(int num)
{
    currentNumMode = num;
}

void Voxelizer::changeCasGridDefinition()
{
    if (gridDefinition == mCascadedGrid.getCascadedLevels() - 1) {
        gridDefinition = 0;
    }
    else {
        gridDefinition += 1;
    }
}

void Voxelizer::setup()
{
    //bindings
    GlobalShaderComponents::bindUBO(voxelMatrixUBOId, GlobalShaderComponents::UBOType::VOXELIZATION_MATRIX);
    GlobalShaderComponents::bindUBO(globalVariablesUBOId, GlobalShaderComponents::UBOType::GLOBAL_VARIABLES);

    ssboCounterSet.bind(GlobalShaderComponents::COUNTER_SSBO_BINDING);
    ssboLogList.bind(GlobalShaderComponents::LOG_SSBO_BINDING);

}

void Voxelizer::dumpCurrentGBuffer()
{
    toDumpCurrentGBuffer = true;
}

void Voxelizer::initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    glCreateTextures(GL_TEXTURE_3D, 1, &textureId);
    glTextureStorage3D(textureId, levels, internalformat, width, height, depth);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}
