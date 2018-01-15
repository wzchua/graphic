#include "SceneMaterialManager.h"



void SceneMaterialManager::addMaterialShapeGroup(const tinyobj::shape_t & shape, const tinyobj::attrib_t & attrib, const tinyobj::material_t & material, const GLuint64 textureIds[4])
{
    ShapeGroup& group = mMatGroupMap[material.name];
    group.addShape(shape, attrib, textureIds, material);

}

void SceneMaterialManager::generateGPUBuffers()
{
    for (auto & g : mMatGroupMap) {
        g.second.generateGPUBuffers();
    }
}

void SceneMaterialManager::render()
{
    for (auto & gIter : mMatGroupMap) {
        gIter.second.render();
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
}
