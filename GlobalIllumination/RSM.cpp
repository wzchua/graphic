#include "RSM.h"

RSM::RSM(glm::ivec2 res) : mRes(res)
{
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

RSM::~RSM()
{
    glDeleteTextures(1, &mDepthMap);
    glDeleteTextures(1, &mVoxelPositionMap);
    glDeleteTextures(1, &mNormalMap);
}
