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
    addAditionalBuffers(GL_RGBA8, "albedoWithShadow");
    addAditionalBuffers(GL_RGBA8, "indirect_illumination");

    glGenFramebuffers(1, &finalFboId);
    glBindFramebuffer(GL_FRAMEBUFFER, finalFboId);
    glCreateTextures(GL_TEXTURE_2D, 1, &mFinalTexture);
    glBindTexture(GL_TEXTURE_2D, mFinalTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, mFinalFormat, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mFinalTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::bindGBuffersAsTexture(GLuint posBinding, GLuint normalBinding, GLuint albedoBinding, GLuint specBinding)
{
    glBindTextureUnit(posBinding, mGBufferTextures[0]);
    glBindTextureUnit(normalBinding, mGBufferTextures[1]);
    glBindTextureUnit(albedoBinding, mGBufferTextures[2]);
    glBindTextureUnit(specBinding, mGBufferTextures[3]);
}

void GBuffer::bindDepthBufferAsTexture(GLuint depthBinding)
{
    glBindTextureUnit(depthBinding, mGBufferTextures[4]);
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

void GBuffer::blitFinalToScreen()
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glBlitNamedFramebuffer(finalFboId, 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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
        std::vector<float> imageFloat;
        image.resize(width * height * 4);
        imageFloat.resize(4 * width * height);
        int i = 0;
        {
            glGetTextureImage(mGBufferTextures[i], 0, GL_RGBA, GL_FLOAT, width * height * 16, imageFloat.data());
            std::string filepath = baseDir + "GBuffer_" + mGBufferNames[i] + ".png";
            glm::vec4 min, max;
            for(int j = 0; j < imageFloat.size(); j+=4) {
                min = glm::min(min, glm::vec4(imageFloat[j], imageFloat[j+1],imageFloat[j+2], imageFloat[j+3]));
                max = glm::max(max, glm::vec4(imageFloat[j], imageFloat[j+1],imageFloat[j+2], imageFloat[j+3]));
            }
            glm::vec4 diff = max-min;
            for(int j = 0; j < imageFloat.size(); j+=4) {
                image[j] = (unsigned char)(imageFloat[j]/diff.x * 255.0f);
                image[j+1] = (unsigned char)(imageFloat[j+1]/diff.y * 255.0f);
                image[j+2] = (unsigned char)(imageFloat[j+2]/diff.z * 255.0f);
                image[j+3] = (unsigned char)(imageFloat[j+3]/diff.w * 255.0f);
            }
            stbi_write_png(filepath.c_str(), width, height, 4, image.data(), 0);
            i++;
        }
        for (; i < MAX_G_BUFFERS - 2; i++) {
            glGetTextureImage(mGBufferTextures[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height * 4, image.data());
            std::string filepath = baseDir + "GBuffer_" + mGBufferNames[i] + ".png";
            stbi_write_png(filepath.c_str(), width, height, 4, image.data(), 0);
        }
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

        {
            glGetTextureImage(mFinalTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height * 4, image.data());
            std::string filepath = baseDir + "GBuffer_Final.png";
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
