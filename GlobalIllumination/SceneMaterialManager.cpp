#include "SceneMaterialManager.h"



void SceneMaterialManager::addMaterialShapeGroup(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const tinyobj::material_t & material, const GLuint64 textureIds[4])
{
    auto const & result = mMatGroupMap.find(material.name);
    GLuint index = -1;
    if (result == mMatGroupMap.end()) {
        index = mGroupList.size();
        mGroupList.emplace_back();
        mMat.emplace_back();
        mMatGroupMap[material.name] = index;
        Mat & mat = mMat[index];
        mat.texAmbient = textureIds[0];
        mat.texDiffuse = textureIds[1];
        mat.texAlpha = textureIds[2];
        mat.texHeight = textureIds[3];
        mat.ambient = { material.ambient[0], material.ambient[1], material.ambient[2], 1.0f };
        mat.diffuse = { material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f };
        mat.specular = { material.specular[0], material.specular[1], material.specular[2], 1.0f };
        mat.shininess = material.shininess;
        mat.useBumpMap = (material.bump_texname.length() > 0) ? 1 : 0;
    }
    else {
        index = result->second;
    }
    
    ShapeGroup& group = mGroupList[index];
    group.addShape(shape, attrib, textureIds, material);

}

void SceneMaterialManager::generateGPUBuffers()
{
    auto matSize = sizeof(Mat);
    //Mat UBO
    for (auto & m : mMat) {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, matSize, &m, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        mMatBufferList.push_back(buffer);
    }

    //VBO, EBO, VAO
    for (auto & g : mGroupList) {
        g.generateGPUBuffers();
    }
}

void SceneMaterialManager::render(GLuint programId)
{
    for (int i = 0; i < mGroupList.size(); i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, mMatBufferList[i]);
        mGroupList[i].render();
    }
}

void SceneMaterialManager::makeAllTextureResident()
{
    for (auto const & t : textureMap) {

        auto handle = glGetTextureHandleARB(t.second);
        glMakeTextureHandleResidentARB(handle);
        mTextureResident.insert(std::make_pair(t.first, handle));
    }
}

void SceneMaterialManager::resetTextureMap()
{
    std::vector<unsigned int> textureIds;
    for (auto const& iter : textureMap) {
        textureIds.push_back(iter.second);
    }
    glDeleteTextures((GLsizei)textureIds.size(), textureIds.data());
    textureMap.clear();
}

void SceneMaterialManager::getTextureHandles(const tinyobj::material_t & material, GLuint64 & texAmbient, GLuint64 & texDiffuse, GLuint64 & texAlpha, GLuint64 & texHeight)
{
    auto textureResult = textureMap.find(material.ambient_texname);
    GLuint64 nullHandle = glGetTextureHandleARB(nullTextureId);
    //ambient
    if (textureResult == textureMap.end()) {
        texAmbient = nullHandle;
    }
    else {
        GLuint textureId = textureResult->second;
        texAmbient = glGetTextureHandleARB(textureId);
    }
    textureResult = textureMap.find(material.diffuse_texname);
    if (textureResult == textureMap.end()) {
        texDiffuse = nullHandle;
    }
    else {
        GLuint textureId = textureResult->second;
        texDiffuse = glGetTextureHandleARB(textureId);
    }
    textureResult = textureMap.find(material.alpha_texname);
    if (textureResult == textureMap.end()) {
        texAlpha = nullHandle;
    }
    else {
        GLuint textureId = textureResult->second;
        texAlpha = glGetTextureHandleARB(textureId);
    }
    textureResult = textureMap.find(material.bump_texname);
    if (textureResult == textureMap.end()) {
        texHeight = nullHandle;
    }
    else {
        GLuint textureId = textureResult->second;
        texHeight = glGetTextureHandleARB(textureId);
    }
}

std::pair<glm::vec3, glm::vec3> SceneMaterialManager::getSceneMinMaxCoords() const
{
    std::pair<glm::vec3, glm::vec3> minmax = mGroupList[0].getMinMaxCoords();

    for (auto & g : mGroupList) {
        auto newMinMax = g.getMinMaxCoords();
        minmax.first = glm::min(minmax.first, newMinMax.first);
        minmax.second = glm::max(minmax.second, newMinMax.second);
    }

    return minmax;
}

SceneMaterialManager::SceneMaterialManager()
{
    int w = 1, h = 1;
    unsigned char image[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &nullTextureId);
    glBindTexture(GL_TEXTURE_2D, nullTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);
    glBindTexture(GL_TEXTURE_2D, 0);

    auto handle = glGetTextureHandleARB(nullTextureId);
    glMakeTextureHandleResidentARB(handle);
}


SceneMaterialManager::~SceneMaterialManager()
{
    glDeleteTextures(1, &nullTextureId);
    resetTextureMap();
    for (int i = 0; i < mGroupList.size(); i++) {
        mGroupList[i].destroyData();
    }
}
