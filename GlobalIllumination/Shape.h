#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <sstream>

#include <tiny_obj_loader.h>
#include "Vertex.h"
//Shape wrapper for tinyobj shape type

class Shape
{
public:
	Shape();
    void buildVBO(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attrib, const tinyobj::material_t & material, glm::vec3 & min, glm::vec3 & max);
    void generateGPUBuffers();
    void render(const GLuint program, const std::map<std::string, GLuint>& textureMap, GLuint nullTextureId) const;
	~Shape();
private:
	std::unordered_map<Vertex, uint32_t> _uniqueVertices = {};
	int _shaderId;
	std::vector<uint32_t> _indices; //EBO
	std::vector<Vertex> _vertexBuffer; //Interleaved, VBO
	unsigned int _vboId, _vaoId, _eboId;
	bool uploadedToGPU = false;
    glm::mat4 _modelMat4 = glm::mat4(1.0f);
    tinyobj::material_t material;
    std::string name;
};