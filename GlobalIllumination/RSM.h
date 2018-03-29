#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <stb_image_write.h>
#include <string>
#include <vector>
class RSM
{
private:
    glm::ivec2 mRes;
    GLuint mDepthMap;
    GLuint mVoxelPositionMap;
    GLuint mNormalMap;
    glm::mat4 shadowMap;
    bool hasInitialized = false;
public:
    //void bindForGeneration(GLuint fbo, GLuint posBinding, GLuint normalBinding);
    void initialize(glm::ivec2 res);
    void dumpAsImage(std::string label);
    GLuint getDepthMap() { return mDepthMap; }
    GLuint getVoxelPositionMap() { return mVoxelPositionMap; }
    GLuint getNormalMap() { return mNormalMap; }
    glm::ivec2 getResolution() { return mRes; };
    void destroyTextures();
    RSM();
    ~RSM();
};

