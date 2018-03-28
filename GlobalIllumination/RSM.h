#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
class RSM
{
private:
    glm::ivec2 mRes;
    GLuint mDepthMap;
    GLuint mVoxelPositionMap;
    GLuint mNormalMap;
public:
    //void bindForGeneration(GLuint fbo, GLuint posBinding, GLuint normalBinding);
    RSM(glm::ivec2 res);
    ~RSM();
    //prevent copying
    RSM(const RSM&) = delete;
    RSM& operator = (const RSM &) = delete;
};

