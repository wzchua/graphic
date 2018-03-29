#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GlobalShaderComponents.h"
#include "AbstractModule.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "LogStruct.h"
#include "CounterBlock.h"
#include "VoxelizeBlock.h"
#include "VoxelizeCascadedBlock.h"
#include "CascadedGrid.h"
#include "OpenGLQueryObject.h"

typedef GlobalShaderComponents GlobalCom;
class RenderToCasGrid : public AbstractModule
{
public:
    void initialize();
    void run(Scene & inputScene, glm::mat4 & worldToVoxelMat, CascadedGrid & casGrid);    
};

