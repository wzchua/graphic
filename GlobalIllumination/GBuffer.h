#pragma once
#include <glad\glad.h>
#include <stb_image_write.h>
#include <vector>

class GBuffer
{
private:
#define MAX_G_BUFFERS 5
    const GLenum attachments[MAX_G_BUFFERS]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_DEPTH_ATTACHMENT };
    GLuint mGBufferTextures[MAX_G_BUFFERS];
    GLenum mGBufferFormat[MAX_G_BUFFERS]{ GL_RGB10_A2, GL_RGB10_A2, GL_RGBA8, GL_R16F, GL_DEPTH_COMPONENT24 };
    std::string mGBufferNames[MAX_G_BUFFERS]{ "voxelpos", "normals", "albedo", "spec", "depth" };
    GLuint width;
    GLuint height;
    GLuint fboId;
    bool hasInitialized = false;
public:
    enum type { VOXEL_POS, NORMAL, ALBEDO, SPECULAR, DEPTH};
    void initialize(GLuint width, GLuint height);
    void bindGBuffersAsTexture(GLuint posBinding, GLuint normalBinding, GLuint albedoBinding, GLuint specBinding);
    GLuint getFboId() { return fboId; }
    void dumpBuffersAsImages();
    GBuffer();
    ~GBuffer();
    //prevent copying
    GBuffer(const GBuffer&) = delete;
    GBuffer& operator = (const GBuffer &) = delete;
};

