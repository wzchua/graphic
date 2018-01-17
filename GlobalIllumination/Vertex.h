#pragma once
#include <glm/gtx/hash.hpp>

/// adapted from vulkan-tutorial.com
/// updated with boost::hash_combine function
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    /*
    GLuint64 texAmbient;
    GLuint64 texDiffuse;
    GLuint64 texAlpha;
    GLuint64 texHeight;
    GLuint useBumpMap = 0;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    GLfloat shininess;*/

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && texCoord == other.texCoord;
        /*
            && texAmbient == other.texAmbient && texDiffuse == other.texDiffuse && texAlpha == other.texAlpha && texHeight == other.texHeight
            && useBumpMap == other.useBumpMap && ambient == other.ambient && diffuse == other.diffuse && specular == other.specular && shininess == other.shininess;*/
    }
};

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            std::size_t h = 0;
            hash_combine(h, vertex.position, vertex.normal, vertex.texCoord);
            /*
                vertex.texAmbient, vertex.texDiffuse, vertex.texAlpha, vertex.texHeight, vertex.useBumpMap, 
                vertex.ambient, vertex.diffuse, vertex.specular, vertex.shininess);*/
            return h;
        }
    };
}