#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <string>

#include "GLBufferObject.h"


class Octree
{
public:
    struct NodeStruct {
        GLuint parentPtr;
        GLuint childPtr;
        GLuint childBit;
        GLuint lightBit;
        GLuint valueIndex;
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
    uint lightBit;
    uint valueIndex;
    uint xPositive;
    uint xNegative;
    uint yPositive;
    uint yNegative;
    uint zPositive;
    uint zNegative;
};
layout(binding = )" + std::to_string(level);
        s = s + R"(, std430) coherent buffer NodeBlock{
    NodeStruct node[];
};
)";
        return s;
    }
    struct NodeValueStruct {
        GLuint color;
        GLuint normal;
        GLuint lightDirection;
        GLuint lightEnergy;
    };
    static glm::vec4 getRGBA(GLuint val) {
        return glm::vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U), float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
    }
    static glm::vec4 getXYZ_Num(GLuint val) {
        glm::vec3 xyz = glm::vec3(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U), float((val & 0x00FF0000) >> 16U));
        xyz = xyz - 128.0f;
        return glm::vec4(xyz, float((val & 0xFF000000) >> 24U));
    }

        static std::string nodeValueStructShaderCodeString(GLuint level) {
            std::string s = R"(struct NodeValueStruct {
    uint color;
    uint normal;
    uint lightDirection;
    uint lightEnergy;
};
layout(binding = )" + std::to_string(level);
            s = s + R"(, std430) coherent buffer NodeValueBlock{
    NodeValueStruct nodeBrick[];
};
)";
            return s;
        }
        static std::string leafStructShaderCodeString(GLuint level) {
            std::string s = R"(layout(binding = )" + std::to_string(level);
            s = s + R"(, std430) coherent buffer LeafListBlock{
    uint leafList[];
};
)";
            return s;
        }

    enum TexType { COLOR, NORMAL, LIGHT_DIRECTION, LIGHT_ENERGY };
    void initialize();
    void resetData();
    GLBufferObject<NodeStruct> & getNodeList();
    GLBufferObject<NodeValueStruct> & getNodeValueList();
    GLBufferObject<GLuint> & getLeafIndexList();
    //GLuint getTextureIds(TexType type);
    Octree();
    ~Octree();
    private:
        unsigned int fragCount = 1024 * 1024 * 2;
        unsigned int nodeCount = 1024 * 1024 * 2;
        GLBufferObject<NodeStruct> ssboNodeList;
        GLBufferObject<NodeValueStruct> ssboNodeValueList;
        GLBufferObject<GLuint> ssboLeafIndexList;


        /*
        int brickDim = 2;
        int texWdith = 512;
        int texHeight = 512;
        GLuint texture3DColorList;
        GLuint texture3DNormalList;
        GLuint texture3DLightDirList;
        GLuint texture3DLightEnergyList;
        void initialize3DTextures(GLuint & textureId, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
        */
};
