#pragma once
#include <glm/gtc/type_ptr.hpp>

#include "Scene.h"
#include "ShaderProgram.h"

class RenderPhong
{
private:
    ShaderProgram phongShader;
public:
    void initialize();
    void run(Scene& scene, GLuint viewWidth, GLuint viewHeight);
    RenderPhong();
    ~RenderPhong();
};

