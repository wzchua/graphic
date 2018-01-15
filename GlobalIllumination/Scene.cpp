#include "Scene.h"



Scene::Scene()
{
}


Scene::~Scene()
{
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
    mSceneMatManager.render();
}

glm::mat4 Scene::getSceneModelMat()
{
    return modelMat;
}

void Scene::updateLightToGPU(int currentShaderProgram)
{
    glUniform4fv(glGetUniformLocation(currentShaderProgram, "LightPosition"), 1, lightPosition);
    glUniform4fv(glGetUniformLocation(currentShaderProgram, "LightAmbient"), 1, lightAmbient);
    glUniform4fv(glGetUniformLocation(currentShaderProgram, "LightDiffuse"), 1, lightDiffuse);
    glUniform4fv(glGetUniformLocation(currentShaderProgram, "LightSpecular"), 1, lightSpecular);
}
