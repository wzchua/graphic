#pragma once
#include "ShaderProgram.h"
#include "CascadedGrid.h"

class FilterCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;

public:
    //void initialize();
    void run(CascadedGrid & cascadedGrid);
    FilterCasGrid();
    ~FilterCasGrid();
};

