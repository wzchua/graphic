#pragma once
#include <map>
#include <string>
#include <tiny_obj_loader.h>

#include "ShapeGroup.h"
class SceneMaterialManager
{
private:
    struct Mat {
        GLuint64 texAmbient;
        GLuint64 texDiffuse;
        GLuint64 texAlpha;
        GLuint64 texHeight;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        int useBumpMap;
        float shininess;
    };
    GLuint nullTextureId;
    std::map<std::string, GLuint> mMatGroupMap;
    std::vector<ShapeGroup> mGroupList;
    std::vector<Mat> mMat;
    std::vector<GLuint> mMatBufferList;
    std::map<std::string, GLuint64> mTextureResident;

    // prevent copying 
    SceneMaterialManager(const SceneMaterialManager&) = delete; // no implementation 
    SceneMaterialManager& operator=(const SceneMaterialManager&) = delete; // no implementation 
public:
    std::map<std::string, GLuint> textureMap;
    void addMaterialShapeGroup(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const tinyobj::material_t & material, const GLuint64 textureIds[4]);
    void generateGPUBuffers();
    void render(GLuint programId);
    void makeAllTextureResident();
    void resetTextureMap();
    void getTextureHandles(const tinyobj::material_t & material, GLuint64 & texAmbient, GLuint64 & texDiffuse, GLuint64 & texAlpha, GLuint64 & texHeight);
    std::pair<glm::vec3, glm::vec3> getSceneMinMaxCoords() const;
    SceneMaterialManager();
    ~SceneMaterialManager();
};

