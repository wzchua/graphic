#pragma once
#include "ShaderProgram.h"
class AbstractModule
{
protected:
    bool hasInitialized = false;
    ShaderProgram shader;
public:
    virtual void initialize() = 0;
    AbstractModule() = default;
    virtual ~AbstractModule() = default;
    //prevent copying
    AbstractModule(const AbstractModule&) = delete;
    AbstractModule& operator = (const AbstractModule &) = delete;
};

