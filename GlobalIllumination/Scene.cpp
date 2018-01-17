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

void Scene::updateMatrixBuffer()
{

    matrixBlock.modelViewMatrix = cam.getViewMatrix() * modelMat;
    matrixBlock.modelViewProjMatrix = cam.getProjMatrix() * matrixBlock.modelViewMatrix;
    matrixBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixBlock.modelViewMatrix))));

    glNamedBufferData(matrixBuffer, sizeof(MatrixBlock), &matrixBlock, GL_DYNAMIC_DRAW);
    /*
    glBindBuffer(GL_UNIFORM_BUFFER, matrixBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixBlock), &matrixBlock, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
}

GLuint Scene::getMatrixBuffer() {
    return matrixBuffer;
}
