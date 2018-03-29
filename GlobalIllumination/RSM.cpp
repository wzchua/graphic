#include "RSM.h"

void RSM::initialize(glm::ivec2 res)
{
    if (hasInitialized) {
        return;
    }
    hasInitialized = true;
    mRes = res;
    const GLfloat texBorder[] = { 1.0f, 0.0f, 0.0f, 0.0f };

    glCreateTextures(GL_TEXTURE_2D, 1, &mDepthMap);
    glBindTexture(GL_TEXTURE_2D, mDepthMap);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, res.x, res.y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texBorder);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &mVoxelPositionMap);
    glBindTexture(GL_TEXTURE_2D, mVoxelPositionMap);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB10_A2, res.x, res.y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &mNormalMap);
    glBindTexture(GL_TEXTURE_2D, mNormalMap);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB10_A2, res.x, res.y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RSM::dumpAsImage(std::string label)
{
    if (hasInitialized) {
        //get win exe path
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        std::string sPath(path);
        auto lastIndex = sPath.find_last_of('\\');
        std::string baseDir = sPath.substr(0, lastIndex + 1);
        int size = mRes.x * mRes.y * 4;
        std::vector<unsigned char> image;
        image.resize(size);

        glGetTextureImage(mVoxelPositionMap, 0, GL_RGBA, GL_UNSIGNED_BYTE, size, image.data());
        std::string filepath = baseDir + "VoxPosRSM_" + label + ".png";
        stbi_write_png(filepath.c_str(), mRes.x, mRes.y, 4, image.data(), 0);

        glGetTextureImage(mNormalMap, 0, GL_RGBA, GL_UNSIGNED_BYTE, size, image.data());
        filepath = baseDir + "NormalRSM_" + label + ".png";
        stbi_write_png(filepath.c_str(), mRes.x, mRes.y, 4, image.data(), 0);

        std::vector<float> imageDepth;
        imageDepth.resize(size);
        glGetTextureImage(mDepthMap, 0, GL_DEPTH_COMPONENT, GL_FLOAT, size, imageDepth.data());
        for (int i = 0; i < image.size(); i++) {
            image[i] = (unsigned char)(imageDepth[i] * 255.0f);
        }
        filepath = baseDir + "DepthRSM_" + label + ".png";
        stbi_write_png(filepath.c_str(), mRes.x, mRes.y, 1, image.data(), 4);
    }
}

void RSM::destroyTextures()
{
    if (hasInitialized) {
        glDeleteTextures(1, &mDepthMap);
        glDeleteTextures(1, &mVoxelPositionMap);
        glDeleteTextures(1, &mNormalMap);
    }
}

RSM::RSM()
{
}

RSM::~RSM()
{
}
