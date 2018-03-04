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

GLuint Scene::getLightBuffer()
{
    return lightBuffer;
}

void Scene::updateLightMatrixBuffer(GLuint index, glm::vec3 forward, glm::vec3 up)
{
    glm::vec3 pos = glm::vec3(light.position.x, light.position.y, light.position.z);
    lightCam.set(pos, forward, up, 100, 1.0f);

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
