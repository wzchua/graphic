#pragma once
#include <glad/glad.h>
#include <string>
#include <map>
#include <vector>

#include "ObjLoader.h"
#include "SceneMaterialManager.h"
#include "Shape.h"
#include "Camera.h"
#include "RSM.h"

class Scene
{
private:
    struct LightBlock {
        glm::vec4 position;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        GLuint rad;
    };
    struct MatrixBlock {
        glm::mat4 modelViewMatrix;
        glm::mat4 modelViewProjMatrix;
        glm::mat4 normalMatrix;
    };
    LightBlock light{ { 10.0f, 150.0f, 100.0f, 1.0f }, { 0.1f, 0.1f, 0.1f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 40000000 };
    struct Light {
        glm::vec4 position;
        glm::vec4 power;
    };
    std::vector<Light> pointLights ={ { {20.0f, 120.0f, 20.0f, 1.0f}, {0.7f, 0.7f, 0.7f, 1.0f} } };
    std::vector<Light> directionalLights;
    std::vector<RSM> pointLightMap;
    std::vector<RSM> directionalLightMap;

    MatrixBlock matrixBlock;
    MatrixBlock matrixLightBlock;
    Camera lightCam = Camera(glm::vec3(light.position.x, light.position.y, light.position.z));
    GLuint matrixBuffer;
    GLuint matrixLightBuffer;
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
    Camera cam = Camera(glm::vec3(10.0f));
    Scene();
    ~Scene();
    bool LoadObjScene(std::string filename);
    void render(int programId);
    glm::mat4 getSceneModelMat();
    RSM& getPointLightRSM(int lightIndex, int face);
    int getTotalPointLights() { return pointLights.size(); }
    GLuint getLightBuffer();
    void updateLightMatrixBuffer(GLuint index, glm::vec3 forward, glm::vec3 up);
    GLuint getLightMatrixBuffer();
    void updateMatrixBuffer();
    GLuint getMatrixBuffer();
    glm::vec3 getSceneMinCoords() { return sceneMin; }
    glm::vec3 getSceneMaxCoords() { return sceneMax; }
};

