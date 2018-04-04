#pragma once
#include <glm\glm.hpp>
#include "AbstractModule.h"
#include "GlobalShaderComponents.h"
#include "CascadedGrid.h"
class CasGridFilter : public AbstractModule
{
private:
    const glm::ivec3 mWorkGroupSize{ 8, 8, 8 };
    ShaderProgram mPreFilterShader;
public:
    void initialize();
    void run(CascadedGrid & casGrid);
};

