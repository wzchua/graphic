#include "Scene.h"



Scene::Scene()
{
}


Scene::~Scene()
{
}

bool Scene::LoadObjScene(std::string filename)
{
    std::vector<unsigned int> textureIds;
    for (auto const& iter : textureMap) {
        textureIds.push_back(iter.second);
    }
    glDeleteTextures(textureIds.size(), textureIds.data());
    textureMap.clear();
    list.clear();
    if (!ObjLoader::loadObj(filename, list, textureMap, sceneMin, sceneMax)) {
        return false;
    }

    for (int i = 0; i < list.size(); i++) {
        list[i].generateGPUBuffers();
    }
    return true;
}

void Scene::render(int programId)
{
    for (int i = 0; i < list.size(); i++) {
        list[i].render(programId, textureMap, nullTextureId);
    }
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
