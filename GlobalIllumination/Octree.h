#pragma once
#include "glad\glad.h"
#include <string>

#include "GLBufferObject.h"


class Octree
{
public:
    struct NodeStruct {
        GLuint parentPtr;
        GLuint childPtr;
        GLuint childBit;
        GLuint modelBrickPtr;
        GLuint lightBit;
        GLuint lightBrickPtr;
        GLuint xPositive;
        GLuint xNegative;
        GLuint yPositive;
        GLuint yNegative;
        GLuint zPositive;
        GLuint zNegative;
    };
    static std::string nodeStructShaderCodeString(GLuint level) {
        std::string s = R"(struct NodeStruct {
    uint parentPtr;
    uint childPtr;
    uint childBit;
    uint modelBrickPtr;
    uint lightBit;
    uint lightBrickPtr;
    uint xPositive;
    uint xNegative;
    uint yPositive;
    uint yNegative;
    uint zPositive;
    uint zNegative;
};
layout(binding = )" + level;
        s = s + R"() coherent buffer NodeBlock{
    NodeStruct node[];
};
)";
        return s;        
    }

    enum TexType { COLOR, NORMAL, LIGHT_DIRECTION, LIGHT_ENERGY };
    void initialize();
    void resetData();
    GLBufferObject<NodeStruct> & getNodeList();
    GLBufferObject<GLuint> & getLeafIndexList();
    GLuint getTextureIds(TexType type);
    Octree();
    ~Octree();
    private:
        unsigned int fragCount = 1024 * 1024 * 2;
        unsigned int nodeCount = 1024 * 1024 * 2;
        int brickDim = 2;
        int texWdith = 512;
        int texHeight = 512;
        GLBufferObject<NodeStruct> ssboNodeList;
        GLBufferObject<GLuint> ssboLeafIndexList;
        GLuint texture3DColorList;
        GLuint texture3DNormalList;
        GLuint texture3DLightDirList;
        GLuint texture3DLightEnergyList;
        void initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
};
