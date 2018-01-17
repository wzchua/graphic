#pragma once
#include <glad/glad.h>
#include <string>
#include <map>
#include <vector>

#include "ObjLoader.h"
#include "SceneMaterialManager.h"
#include "Shape.h"
#include "Camera.h"

class Scene
{
private:
    struct LightBlock {
        glm::vec4 position;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
    };
    struct MatrixBlock {
        glm::mat4 modelViewMatrix;
        glm::mat4 modelViewProjMatrix;
        glm::mat4 normalMatrix;
    };
    LightBlock light{ { 10.0f, 5.0f, 10.0f, 0.0f }, { 0.1f, 0.1f, 0.1f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
    MatrixBlock matrixBlock;
    GLuint matrixBuffer;
    GLuint lightBuffer;
    std::map<std::string, GLuint> textureMap;
    std::vector<Shape> list;
    glm::mat4 modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.2, 0.2, 0.2));
    glm::vec3 sceneMin;
    glm::vec3 sceneMax;
    SceneMaterialManager mSceneMatManager;
    // copy ops are private to prevent copying 
    Scene(const Scene&); // no implementation 
    Scene& operator=(const Scene&); // no implementation 
public:
    Camera cam = Camera(glm::vec3(5.0f));
    Scene();
    ~Scene();
    bool LoadObjScene(std::string filename);
    void render(int programId);
    glm::mat4 getSceneModelMat();
    GLuint getLightBuffer();
    void updateMatrixBuffer();
    GLuint getMatrixBuffer();
    glm::vec3 getSceneMinCoords() { return sceneMin; }
    glm::vec3 getSceneMaxCoords() { return sceneMax; }
};

