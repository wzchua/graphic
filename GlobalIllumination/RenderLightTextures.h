#pragma once
#include "GlobalShaderComponents.h"
#include "AbstractModule.h"
#include "Scene.h"
#include "VoxelizeBlock.h"
class RenderLightTextures : public AbstractModule
{
private:
    GLuint fboId;
    const glm::ivec2 rsmRes{ 1024, 1024 };
    glm::vec3 forward[6]{ glm::vec3(1, 0, 0) , glm::vec3(-1, 0, 0),  glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) , glm::vec3(0, 1, 0) , glm::vec3(0, -1, 0) };
    glm::vec3 up[6]{ glm::vec3(0, 1, 0) , glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) , glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0) };
    const GLenum attachments[2]{ GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT1 };
public:
    void initialize();
    void run(Scene& inputScene);
};

