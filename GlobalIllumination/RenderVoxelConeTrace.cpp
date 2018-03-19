#include "RenderVoxelConeTrace.h"



void RenderVoxelConeTrace::initialize()
{
    shader.generateShader("./Shaders/VoxelConeTracingRender.vert", ShaderProgram::VERTEX);
    shader.generateShader("./Shaders/VoxelConeTracingRender.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();

    //cube vao
    std::vector<glm::vec3> quadVertices;
    quadVertices.push_back(glm::vec3(1.0, 1.0, 0.0));
    quadVertices.push_back(glm::vec3(1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
    quadVertices.push_back(glm::vec3(-1.0, 1.0, 0.0));

    std::vector<GLuint> quadindices;
    quadindices.push_back(0);
    quadindices.push_back(3);
    quadindices.push_back(1);
    quadindices.push_back(2);
    quadindices.push_back(1);
    quadindices.push_back(3);

    int vertexSize = sizeof(glm::vec3);
    glGenVertexArrays(1, &quadVAOId);
    glBindVertexArray(quadVAOId);

    glGenBuffers(1, &quadVBOId);
    glGenBuffers(1, &quadEBOId);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBOId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadindices.size() * sizeof(GLuint), quadindices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBOId);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * vertexSize, quadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RenderVoxelConeTrace::run(Scene & inputScene, GLBufferObject<CounterBlock>& counterBlk, Octree & octree)
{
    shader.use();
    glViewport(0, 0, 800, 600); // light render is done at 1024x1024
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    /*
    glBindTextureUnit(4, octree.getTextureIds(Octree::COLOR));
    glBindTextureUnit(5, octree.getTextureIds(Octree::NORMAL));
    glBindTextureUnit(6, octree.getTextureIds(Octree::LIGHT_DIRECTION));
    glBindTextureUnit(7, octree.getTextureIds(Octree::LIGHT_ENERGY));
    */

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getMatrixBuffer()); //scene cam matrices
    counterBlk.bind(1);
    octree.getNodeList().bind(2);
    octree.getNodeValueList().bind(3);

    inputScene.render(shader.getProgramId());
}

RenderVoxelConeTrace::RenderVoxelConeTrace()
{
}


RenderVoxelConeTrace::~RenderVoxelConeTrace()
{
}
