#include "Scene.h"



Scene::Scene()
{
    glGenBuffers(1, &lightBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBlock), &light, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &matrixBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixBlock), &matrixBlock, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &matrixLightBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, matrixLightBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixBlock), &matrixLightBlock, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    pointLightMap.resize(pointLights.size() * 6);
    for (int i = 0; i < pointLights.size(); i++) {
        GLuint uboId;
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), &pointLights[i], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        pointLightBuffers.push_back(uboId);
    }
}


Scene::~Scene()
{
    glDeleteBuffers(1, &lightBuffer);
}

bool Scene::LoadObjScene(std::string filename)
{
    mSceneMatManager.resetTextureMap();
    list.clear();
    if (!ObjLoader::loadObj(filename, mSceneMatManager, sceneMin, sceneMax)) {
        return false;
    }

    mSceneMatManager.generateGPUBuffers();
    mSceneMatManager.makeAllTextureResident();
    auto minmax = mSceneMatManager.getSceneMinMaxCoords();
    sceneMin = minmax.first;
    sceneMax = minmax.second;
    return true;
}

void Scene::render(int programId)
{
    mSceneMatManager.render(programId);
}

glm::mat4 Scene::getSceneModelMat()
{
    return modelMat;
}

RSM & Scene::getPointLightRSM(int lightIndex, int face)
{
    return pointLightMap[lightIndex * 6 + face];
}

GLuint Scene::getPointLightBufferId(int lightIndex)
{
    return pointLightBuffers[lightIndex];
}

GLuint Scene::getLightBuffer()
{
    return lightBuffer;
}

void Scene::updateLightMatrixBuffer(GLuint index, glm::vec3 forward, glm::vec3 up)
{    
    Light& selected = pointLights[index];
    glm::vec3 pos = glm::vec3(selected.position.x, selected.position.y, selected.position.z);
    lightCam.set(pos, forward, up, 90.0f, glm::ivec2(1024, 1024));

    matrixLightBlock.modelViewMatrix = lightCam.getViewMatrix() * modelMat;
    matrixLightBlock.modelViewProjMatrix = lightCam.getProjMatrix() * matrixLightBlock.modelViewMatrix;
    matrixLightBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixLightBlock.modelViewMatrix))));

    glNamedBufferSubData(matrixLightBuffer, 0, sizeof(MatrixBlock), &matrixLightBlock);
}

GLuint Scene::getLightMatrixBuffer()
{
    return matrixLightBuffer;
}

void Scene::updateMatrixBuffer()
{

    matrixBlock.modelViewMatrix = cam.getViewMatrix() * modelMat;
    matrixBlock.modelViewProjMatrix = cam.getProjMatrix() * matrixBlock.modelViewMatrix;
    matrixBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixBlock.modelViewMatrix))));

    glNamedBufferSubData(matrixBuffer, 0, sizeof(MatrixBlock), &matrixBlock);
}

GLuint Scene::getMatrixBuffer() {
    return matrixBuffer;
}

std::string Scene::getLightUBOCode(int lightBinding)
{
    std::string s = R"*(layout(binding = )*" + std::to_string(lightBinding)
        + R"*(, std140) uniform LightBlock{ 
    vec4 LightPosition; // Given in world space. Can be directional.
    vec4 LightPower;
};
)*";
    return s;
}
