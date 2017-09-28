#include "Renderer.h"

void Renderer::setToPhongShader()
{
    type = RenderType::PHONG;
    currentShaderProgram = phongShader.use();

    glViewport(0, 0, viewWidth, viewHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    scene.updateLightToGPU(currentShaderProgram);
}

void Renderer::phongRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 modelViewMat = scene.cam.getViewMatrix() * scene.getSceneModelMat();
    glm::mat4 modelViewProjMat = scene.cam.getProjMatrix() * modelViewMat;
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelViewMat)));

    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glUniformMatrix3fv(glGetUniformLocation(currentShaderProgram, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMat));

    scene.render(currentShaderProgram);

    unsigned int fragmentCount = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    fragmentCount = ptr[0];
    ptr[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void Renderer::voxelRender() {
    glViewport(0, 0, 512, 512);
    //warp scene into ortho box 512 x 512 x 512
    glm::vec3 min = scene.getSceneMinCoords();
    glm::vec3 max = scene.getSceneMaxCoords();
    glm::vec3 length = (max - min);
    glm::vec3 translate = glm::vec3(0.0f) - (min + length / 2.0f);
    glm::vec3 scale = 512.0f / length;
    glm::mat4 sceneMat = glm::translate(glm::scale(glm::mat4(1.0f), scale), translate);
    glm::vec3 newMin = sceneMat * glm::vec4(min, 1.0f);

    glm::mat4 ortho = glm::ortho(-256.0, 256.0, -256.0, 256.0, 0.0, 512.0);
    glm::mat4 voxelViewMatrix = glm::lookAt(glm::vec3(256, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 modelViewMat = voxelViewMatrix * sceneMat;
    glm::mat4 modelViewProjMat = ortho * modelViewMat;
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMat));
    glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "ModelViewProjMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjMat));
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    scene.render(currentShaderProgram);
    // get length of fragment list
    unsigned int fragmentCount = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    fragmentCount = ptr[0];
    ptr[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    //std::cout << fragmentCount << " fragments" << std::endl;

    //build octree
}

void Renderer::setToVoxelizeShader() {
    type = RenderType::VOXELIZE;
    currentShaderProgram = voxelizeListShader.use();
}

Renderer::Renderer(GLFWwindow * window, unsigned int viewHeight, unsigned int viewWidth) : viewHeight(viewHeight), viewWidth(viewWidth)
{
    this->window = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, onKeyStatic);
    glfwSetCursorPosCallback(window, onCursorPositionStatic);
    phongShader.generateShader("./Shaders/Phong.vert", ShaderProgram::VERTEX);
    phongShader.generateShader("./Shaders/Phong.frag", ShaderProgram::FRAGMENT);
    phongShader.linkCompileValidate();

    voxelizeListShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeListShader.generateShader("./Shaders/Voxelize.frag", ShaderProgram::FRAGMENT);
    voxelizeListShader.linkCompileValidate();

    octreeCompShader.generateShader("./Shaders/BuildOctree.comp", ShaderProgram::COMPUTE);
    octreeCompShader.linkCompileValidate();




    //atomic counter buffer
    glGenBuffers(1, &atomicBuffer);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);


    struct fragStruct {
        float position[4];
        float color[4];
    };
    unsigned int count = 512 * 256 * 256;
    GLint size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
    std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << size << " bytes." << std::endl;
    std::cout << "wanting to set is " << sizeof(fragStruct) * count << " bytes." << std::endl;
    //fragmentlist buffer
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(fragStruct) * count, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    
    int w = 1, h = 1;
    unsigned char image[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &nullTextureId);
    glBindTexture(GL_TEXTURE_2D, nullTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);
    glBindTexture(GL_TEXTURE_2D, 0);
    scene.nullTextureId = nullTextureId;

    setToPhongShader();
    //setToVoxelizeShader();
}

Renderer::~Renderer()
{
    glDeleteTextures(1, &nullTextureId);
}

void Renderer::startRenderLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        switch (type)
        {
        case Renderer::PHONG:
            phongRender();
            break;
        case Renderer::VOXELIZE:
            voxelRender();
            break;
        default:
            break;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::loadScene(std::string filename)
{
    scene.LoadObjScene(filename);
}

void Renderer::onCursorPosition(GLFWwindow* window, double xpos, double ypos) {
    scene.cam.viewMove(xpos, ypos);
    glfwSetCursorPos(window, 0.0, 0.0);
}

void Renderer::onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Close window
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        glm::vec3 camMove;
        if (key == GLFW_KEY_W) {
            camMove.z += 1.0f;
        }
        if (key == GLFW_KEY_S) {
            camMove.z -= 1.0f;
        }
        if (key == GLFW_KEY_A) {
            camMove.x += 1.0f;
        }
        if (key == GLFW_KEY_D) {
            camMove.x -= 1.0f;
        }
        // doesn't register stimulateous presses
        if (camMove.z != 0.0f || camMove.x != 0.0f) {
            scene.cam.move(camMove);
        }
    }
}