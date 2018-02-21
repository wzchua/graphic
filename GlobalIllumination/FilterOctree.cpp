#include "FilterOctree.h"



void FilterOctree::run(GLBufferObject<NodeStruct>& nodeOctree, GLBufferObject<GLuint> & ssboLeafIndexList, GLuint textureColor, GLuint textureNormal)
{
    glBindImageTexture(4, textureColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, textureNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
}

FilterOctree::FilterOctree()
{
}


FilterOctree::~FilterOctree()
{
}
