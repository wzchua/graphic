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
    const GLfloat lightPosition[4] = { 10.0f, 5.0f, 10.0f, 0.0f };   // Directional light. Given in eye space.
    const GLfloat lightAmbient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    const GLfloat lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
    void updateLightToGPU(int currentShaderProgram);
    glm::vec3 getSceneMinCoords() { return sceneMin; }
    glm::vec3 getSceneMaxCoords() { return sceneMax; }
};

