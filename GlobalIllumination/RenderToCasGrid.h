#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "LogStruct.h"
#include "CounterBlock.h"
#include "VoxelizeBlock.h"
#include "VoxelizeCascadedBlock.h"
#include "CascadedGrid.h"
class RenderToCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;
    //prevent copying
    RenderToCasGrid(const RenderToCasGrid&) = delete;
    RenderToCasGrid& operator = (const RenderToCasGrid &) = delete;
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, glm::mat4 & worldToVoxelMat, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, CascadedGrid & casGrid);
    RenderToCasGrid();
    ~RenderToCasGrid();
};
