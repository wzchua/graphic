#include "GBuffer.h"



void GBuffer::initialize(GLuint width, GLuint height)
{
    if (hasInitialized) {
        return;
    }
    stbi_flip_vertically_on_write(true);
    hasInitialized = true;
    this->width = width;
    this->height = height;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    for (int i = 0; i < MAX_G_BUFFERS; i++) {
        
        glCreateTextures(GL_TEXTURE_2D, 1, &mGBufferTextures[i]);
        glBindTexture(GL_TEXTURE_2D, mGBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, mGBufferFormat[i], width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glFramebufferTexture(GL_FRAMEBUFFER, attachments[i], mGBufferTextures[i], 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glDrawBuffers(MAX_G_BUFFERS - 1, attachments); //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    addAditionalBuffers(GL_RGBA8, "indirect_illumination");
    addAditionalBuffers(GL_RGBA8, "final");
}

void GBuffer::bindGBuffersAsTexture(GLuint posBinding, GLuint normalBinding, GLuint albedoBinding, GLuint specBinding)
{
    glBindTextureUnit(posBinding, mGBufferTextures[0]);
    glBindTextureUnit(normalBinding, mGBufferTextures[1]);
    glBindTextureUnit(albedoBinding, mGBufferTextures[2]);
    glBindTextureUnit(specBinding, mGBufferTextures[3]);
}

GLuint GBuffer::addAditionalBuffers(GLenum format, std::string name)
{
    GLuint bufferId;
    glCreateTextures(GL_TEXTURE_2D, 1, &bufferId);
    glBindTexture(GL_TEXTURE_2D, bufferId);
    glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);
    additionalBuffers.push_back(bufferId);
    additionalBufferFormats.push_back(format);
    additionalBufferNames.push_back(name);
    return bufferId;
}

void GBuffer::dumpBuffersAsImages()
{
    if (hasInitialized) {
        //get win exe path
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        std::string sPath(path);
        auto lastIndex = sPath.find_last_of('\\');
        std::string baseDir = sPath.substr(0, lastIndex + 1);

        std::vector<unsigned char> image;
        image.resize(width * height * 4);
        int i = 0;
        for (; i < MAX_G_BUFFERS - 2; i++) {
            glGetTextureImage(mGBufferTextures[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height * 4, image.data());
            std::string filepath = baseDir + "GBuffer_" + mGBufferNames[i] + ".png";
            stbi_write_png(filepath.c_str(), width, height, 4, image.data(), 0);
        }
        std::vector<float> imageFloat;
        imageFloat.resize(4 * width * height);
        glGetTextureImage(mGBufferTextures[i], 0, GL_RED, GL_FLOAT, width * height * 4, imageFloat.data());
        std::string filepath = baseDir + "GBuffer_" + mGBufferNames[i] + ".png";
        for (int i = 0; i < image.size(); i++) {
            image[i] = (unsigned char)(imageFloat[i]/ 20.0f * 255.0f);
        }
        stbi_write_png(filepath.c_str(), width, height, 1, image.data(), 4);

        i++;
        std::vector<unsigned int> imageDepth;
        imageDepth.resize(4 * width * height);
        glGetTextureImage(mGBufferTextures[i], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4 * width * height, imageDepth.data());
        for (int i = 0; i < image.size(); i++) {
            image[i] = (unsigned char)(imageDepth[i] & 0xFFFFFF >> 16);
        }
        filepath = baseDir + "GBuffer_" + mGBufferNames[i] + ".png";
        stbi_write_png(filepath.c_str(), width, height, 1, image.data(), 4);

        image.resize(width * height * 4);        
        for (int j = 0; j < additionalBuffers.size(); j++) {
            glGetTextureImage(additionalBuffers[j], 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height * 4, image.data());
            std::string filepath = baseDir + "GBuffer_" + additionalBufferNames[j] + ".png";
            stbi_write_png(filepath.c_str(), width, height, 4, image.data(), 0);
        }

    }
}

GBuffer::GBuffer()
{
}


GBuffer::~GBuffer()
{
    if (hasInitialized) {
        for (int i = 0; i < MAX_G_BUFFERS; i++) {
            glDeleteTextures(1, &mGBufferTextures[i]);
        }
    }
}
