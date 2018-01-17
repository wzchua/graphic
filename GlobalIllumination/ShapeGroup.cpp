#include "ShapeGroup.h"
#include <iostream>
void CheckGLError2()
{
    GLenum err(glGetError());

    while (err != GL_NO_ERROR)
    {
        std::string error;
        switch (err)
        {
        case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        std::cout << "GL_" << error.c_str() << std::endl;
        err = glGetError();
    }

    return;
}

void ShapeGroup::addShape(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const GLuint64 textureIds[4], tinyobj::material_t const & mat)
{
    std::vector<GLuint> shapeIndices;
    shapeIndices.reserve(shape.mesh.indices.size());
    GLvoid * start = (GLvoid *) mIndices.size();
    for (const auto& index : shape.mesh.indices) {
        Vertex vertex;
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]
        };
        min = glm::min(vertex.position, min);
        max = glm::max(vertex.position, max);
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]
        };
        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1]
        };
        /*
        vertex.texAmbient = textureIds[0];
        vertex.texDiffuse = textureIds[1];
        vertex.texAlpha = textureIds[2];
        vertex.texHeight = textureIds[3];
        vertex.useBumpMap = (mat.bump_texname.length() > 0) ? 1 : 0;
        vertex.ambient = {
            mat.ambient[0], mat.ambient[1], mat.ambient[2]
        };
        vertex.diffuse = {
            mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]
        };
        vertex.specular = {
            mat.specular[0], mat.specular[1], mat.specular[2]
        };
        vertex.shininess = mat.shininess;*/

        if (mUniqueVertices.count(vertex) == 0) {
            mUniqueVertices[vertex] = static_cast<GLuint>(mVertexBuffer.size());
            mVertexBuffer.push_back(vertex);
        }
        mIndices.push_back(mUniqueVertices[vertex]);
        shapeIndices.push_back(mIndices.back());
    }
    mListOfShapeStarts.push_back(start);
    mListOfShapeIndices.push_back(shapeIndices);
    mListOfShapeCounts.push_back(shapeIndices.size());
    textureHandles[0] = textureIds[0];
    textureHandles[1] = textureIds[1];
    textureHandles[2] = textureIds[2];
    textureHandles[3] = textureIds[3];
}

void ShapeGroup::generateGPUBuffers()
{
    if (mIndices.size() == 0) {
        return;
    }
    if (mUploadedToGPU) {
        return;
    }
    glGenVertexArrays(1, &mVAOId);
    glBindVertexArray(mVAOId);

    glGenBuffers(1, &mVBOId);
    glGenBuffers(1, &mEBOId);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBOId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), mIndices.data(), GL_DYNAMIC_DRAW);

    GLsizei vertexSize = sizeof(Vertex);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOId);
    glBufferData(GL_ARRAY_BUFFER, mVertexBuffer.size() * vertexSize, mVertexBuffer.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Vertex::position));
    glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Vertex::normal));
    glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, Vertex::texCoord));
    glVertexAttribBinding(0, 0);
    glVertexAttribBinding(1, 0);
    glVertexAttribBinding(2, 0);
    glBindVertexBuffer(0, mVBOId, 0, vertexSize);
    
    /*
    glBindBuffer(GL_ARRAY_BUFFER, mVBOId);
    glBufferData(GL_ARRAY_BUFFER, mVertexBuffer.size() * vertexSize, mVertexBuffer.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texCoord));
    glEnableVertexAttribArray(2);*/
    
    /*
    glVertexAttribLPointer(3, 1, GL_UNSIGNED_INT64_ARB, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texAmbient));
    glEnableVertexAttribArray(3);

    glVertexAttribLPointer(4, 1, GL_UNSIGNED_INT64_ARB, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texDiffuse));
    glEnableVertexAttribArray(4);

    glVertexAttribLPointer(5, 1, GL_UNSIGNED_INT64_ARB, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texAlpha));
    glEnableVertexAttribArray(5);

    glVertexAttribLPointer(6, 1, GL_UNSIGNED_INT64_ARB, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texHeight));
    glEnableVertexAttribArray(6);

    glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::useBumpMap));
    glEnableVertexAttribArray(7);

    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::ambient));
    glEnableVertexAttribArray(8);

    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::diffuse));
    glEnableVertexAttribArray(9);

    glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::specular));
    glEnableVertexAttribArray(10);

    glVertexAttribPointer(11, 1, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::shininess));
    glEnableVertexAttribArray(11);*/

    mUploadedToGPU = true;
    glBindVertexArray(0);
}

void ShapeGroup::render() const
{
    //CheckGLError2();
    glBindVertexArray(mVAOId);
    //glMultiDrawElements(GL_TRIANGLES, mListOfShapeCounts.data(), GL_UNSIGNED_INT, mListOfShapeStarts.data(), mListOfShapeCounts.size());
    glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    //CheckGLError2();
}

std::pair<glm::vec3, glm::vec3> ShapeGroup::getMinMaxCoords() const
{
    return std::pair<glm::vec3, glm::vec3>(min, max);
}

void ShapeGroup::destroyData()
{
    if (mUploadedToGPU) {
        glDeleteBuffers(1, &mVBOId);
        glDeleteVertexArrays(1, &mVAOId);
    }
}

ShapeGroup::ShapeGroup()
{
}


ShapeGroup::~ShapeGroup()
{
}
