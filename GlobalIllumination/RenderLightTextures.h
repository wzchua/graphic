#pragma once
#include "AbstractModule.h"
#include "Scene.h"
class RenderLightTextures : public AbstractModule
{
private:
    GLuint rboId;
    GLuint fboId;
    const glm::ivec2 rsmRes{ 1024, 1024 };
    glm::vec3 forward[6]{ glm::vec3(1, 0, 0) , glm::vec3(-1, 0, 0),  glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) , glm::vec3(0, 1, 0) , glm::vec3(0, -1, 0) };
    glm::vec3 up[6]{ glm::vec3(0, 1, 0) , glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) , glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 0) };
public:
    void initialize();
    void run(Scene& inputScene, glm::ivec2 res, std::vector<GLuint> & depthMap, std::vector<GLuint> & positionMap, std::vector<GLuint> & normalMap);
};

