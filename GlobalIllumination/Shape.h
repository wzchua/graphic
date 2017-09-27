#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <sstream>

#include <tiny_obj_loader.h>
//Shape wrapper for tinyobj shape type


/// adapted from vulkan-tutorial.com
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
    glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return position == other.position && normal == other.normal && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

class Shape
{
public:
	Shape();
    void buildVBO(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attrib, const tinyobj::material_t & material);
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