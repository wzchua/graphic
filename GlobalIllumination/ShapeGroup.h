#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <tiny_obj_loader.h>

#include "Vertex.h"

class ShapeGroup
{
private:
    std::unordered_map<Vertex, GLuint> mUniqueVertices = {};
    int mShaderId;
    std::vector<GLuint> mIndices; //EBO
    std::vector<Vertex> mVertexBuffer; //Interleaved, VBO
    std::vector<std::vector<GLuint>> mListOfShapeIndices;
    std::vector<GLsizei> mListOfShapeCounts;
    std::vector<GLvoid *> mListOfShapeStarts;
    unsigned int mVBOId, mVAOId, mEBOId;
    bool mUploadedToGPU = false;
    glm::vec3 min;
    glm::vec3 max;
    GLuint64 textureHandles[4];
public:
    void addShape(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const GLuint64 textureIds[4], tinyobj::material_t const & mat);
    void generateGPUBuffers();
    void render() const;
    std::pair<glm::vec3, glm::vec3> getMinMaxCoords() const;
    void destroyData();
    ShapeGroup();
    ~ShapeGroup();
};

