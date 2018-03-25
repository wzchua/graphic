#pragma once
#include <glm/gtc/type_ptr.hpp>

#include "Scene.h"
#include "ShaderProgram.h"

class RenderPhong
{
private:
    ShaderProgram phongShader;
    int num = 1;
public:
    void initialize();
    void setup(Scene & scene, GLuint viewWidth, GLuint viewHeight);
    void run(Scene& scene);
    void onNumberPressed(int num);
    RenderPhong();
    ~RenderPhong();
};

