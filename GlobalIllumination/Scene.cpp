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

    directionalLightMap.resize(directionalLights.size());
    for (int i = 0; i < directionalLights.size(); i++) {
        GLuint uboId;
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), &directionalLights[i], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        directionalLightBuffers.push_back(uboId);
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

RSM & Scene::getDirectionalLightRSM(int lightIndex)
{
    return directionalLightMap[lightIndex];
}

GLuint Scene::getPointLightBufferId(int lightIndex)
{
    return pointLightBuffers[lightIndex];
}

GLuint Scene::getDirectionalLightBufferId(int lightIndex)
{
    return directionalLightBuffers[lightIndex];
}

GLuint Scene::getLightBuffer()
{
    return lightBuffer;
}

void Scene::updateLightMatrixBufferForPointLight(GLuint index, glm::vec3 forward, glm::vec3 up)
{    
    Light& selected = pointLights[index];
    glm::vec3 pos = glm::vec3(selected.position.x, selected.position.y, selected.position.z);
    lightCam.set(pos, forward, up, 90.0f, glm::ivec2(1024, 1024));

    matrixLightBlock.modelViewMatrix = lightCam.getViewMatrix() * modelMat;
    matrixLightBlock.modelViewProjMatrix = lightCam.getProjMatrix() * matrixLightBlock.modelViewMatrix;
    matrixLightBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixLightBlock.modelViewMatrix))));

    glNamedBufferSubData(matrixLightBuffer, 0, sizeof(MatrixBlock), &matrixLightBlock);
}

void Scene::updateLightMatrixBufferForDirectionalLight(GLuint index)
{
    glm::vec3 diff = sceneMax - sceneMin;
    glm::vec3 center = (diff) / 2.0f + sceneMin;
    glm::vec3 dir = glm::normalize(glm::vec3(directionalLights[index].position));
    glm::vec3 pos = center - dir * 100.0f;
    glm::vec3 up = glm::normalize(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), dir));
    if (glm::abs(glm::dot(up, dir)) == 1.0f) {
        up = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), dir));
    }
    /*glm::vec3 z = normalize(dir);
    glm::vec3 x = normalize(cross(z, up));
    glm::vec3 y = cross(x, z);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    viewMatrix[0][0] = x.x;
    viewMatrix[1][0] = x.y;
    viewMatrix[2][0] = x.z;
    viewMatrix[0][1] = y.x;
    viewMatrix[1][1] = y.y;
    viewMatrix[2][1] = y.z;
    viewMatrix[0][2] =-z.x;
    viewMatrix[1][2] =-z.y;
    viewMatrix[2][2] =-z.z;*/
     
    glm::mat4 viewMatrix = glm::mat3(glm::lookAt(pos,
        center,
        up));
    glm::vec4 min = viewMatrix * glm::vec4(sceneMin, 1.0f);
    glm::vec4 max = viewMatrix * glm::vec4(sceneMax, 1.0f);
    glm::vec3 newMin = glm::min(min, max);
    glm::vec3 newMax = glm::max(min, max);
    glm::mat4 projMatrix = glm::ortho(newMin.x/2.0f, newMax.x / 2.0f, newMin.y / 2.0f, newMax.y / 2.0f, 0.0f, 4000.0f);
    matrixLightBlock.modelViewMatrix = viewMatrix * modelMat;
    matrixLightBlock.modelViewProjMatrix = projMatrix * matrixLightBlock.modelViewMatrix;
    matrixLightBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixLightBlock.modelViewMatrix))));

/*
    lightCam.set(pos, dir, up, 90.0f, glm::ivec2(1024, 1024));
    matrixLightBlock.modelViewMatrix = lightCam.getViewMatrix() * modelMat;
    matrixLightBlock.modelViewProjMatrix = lightCam.getProjMatrix() * matrixLightBlock.modelViewMatrix;
    matrixLightBlock.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(matrixLightBlock.modelViewMatrix))));*/

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
