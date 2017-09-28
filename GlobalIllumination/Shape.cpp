#include "Shape.h"
#include <iostream>
Shape::Shape()
{
}

void Shape::buildVBO(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const tinyobj::material_t & material, glm::vec3 & min, glm::vec3 & max)
{
    name = shape.name;
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
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
        };

		if (_uniqueVertices.count(vertex) == 0) {
			_uniqueVertices[vertex] = static_cast<uint32_t>(_vertexBuffer.size());
			_vertexBuffer.push_back(vertex);
		}
		_indices.push_back(_uniqueVertices[vertex]);
	}
    this->material = material;
}

void Shape::generateGPUBuffers()
{
    if (uploadedToGPU) {
        return;
    }
    glGenVertexArrays(1, &_vaoId);
    glBindVertexArray(_vaoId);

    glGenBuffers(1, &_vboId);
    glGenBuffers(1, &_eboId);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), _indices.data(), GL_STATIC_DRAW); //GL_DYNAMIC_DRAW

    auto vertexSize = sizeof(Vertex);
    glBindBuffer(GL_ARRAY_BUFFER, _vboId);
    glBufferData(GL_ARRAY_BUFFER, _vertexBuffer.size() * vertexSize, _vertexBuffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)offsetof(Vertex, Vertex::texCoord));
    glEnableVertexAttribArray(2);

    uploadedToGPU = true;
    glBindVertexArray(0);
}

void Shape::render(const GLuint program, const std::map<std::string, GLuint>& textureMap, GLuint nullTextureId) const {
    GLuint texId;
    auto iter = textureMap.find(material.ambient_texname);
    std::stringstream str;
    str << name << ": ";

    glActiveTexture(GL_TEXTURE0);
    if (iter != textureMap.end()) {
        texId = iter->second;
        glBindTexture(GL_TEXTURE_2D, texId);
        str << material.ambient_texname << ", ";
    }
    else {
        glBindTexture(GL_TEXTURE_2D, nullTextureId);
    }
    glActiveTexture(GL_TEXTURE1);
    iter = textureMap.find(material.diffuse_texname);
    if (iter != textureMap.end()) {
        texId = iter->second;
        glBindTexture(GL_TEXTURE_2D, texId);
        str << material.diffuse_texname << ", ";
    }
    else {
        glBindTexture(GL_TEXTURE_2D, nullTextureId);
    }
    glActiveTexture(GL_TEXTURE2);
    iter = textureMap.find(material.alpha_texname);
    if (iter != textureMap.end()) {
        texId = iter->second;
        glBindTexture(GL_TEXTURE_2D, texId);
        str << material.alpha_texname << ", ";
    }
    else {
        glBindTexture(GL_TEXTURE_2D, nullTextureId);
    }
    glActiveTexture(GL_TEXTURE3);
    iter = textureMap.find(material.bump_texname);
    if (iter != textureMap.end()) {
        texId = iter->second;
        glBindTexture(GL_TEXTURE_2D, texId);
        glUniform1f(glGetUniformLocation(program, "useBumpMap"), true);
        str << material.bump_texname << ", ";
    }
    else {
        glUniform1f(glGetUniformLocation(program, "useBumpMap"), false);
    }
    glActiveTexture(GL_TEXTURE4);
    iter = textureMap.find(material.specular_texname);
    if (iter != textureMap.end()) {
        texId = iter->second;
        glBindTexture(GL_TEXTURE_2D, texId);
        str << material.specular_texname << ", ";
    }
    else {
        glBindTexture(GL_TEXTURE_2D, nullTextureId);
    }
    //std::cout << str.str() << "\n";
    glUniform3fv(glGetUniformLocation(program, "Ambient"), 1, material.ambient);
    glUniform3fv(glGetUniformLocation(program, "Diffuse"), 1, material.diffuse);
    glUniform3fv(glGetUniformLocation(program, "Specular"), 1, material.specular);
    glUniform1f(glGetUniformLocation(program, "MatlShininess"), material.shininess);

    glBindVertexArray(_vaoId);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Shape::~Shape()
{
    if (uploadedToGPU) {
        glDeleteBuffers(1, &_vboId);
        glDeleteVertexArrays(1, &_vaoId);
    }
}
