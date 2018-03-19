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
layout(binding = , std430)" + level;
        s = s + R"() coherent buffer NodeBlock{
    NodeStruct node[];
};
)";
        return s;
    }
    struct NodeValueStruct {
        glm::vec4 color;
        glm::vec4 normal;
        glm::vec3 lightDirection;
        GLuint lightEnergy;
    };

        static std::string nodeValueStructShaderCodeString(GLuint level) {
            std::string s = R"(struct NodeValueStruct {
    vec4 color;
    vec4 normal;
    vec3 lightDirection;
    uint lightEnergy;
};
layout(binding = , std430)" + level;
            s = s + R"() coherent buffer NodeBlock{
    NodeValueStruct nodeBrick[];
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
