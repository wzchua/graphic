#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <cstdlib>

#include "Renderer.h"
#include "Scene.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
std::string filepath;

static void WaitForEnterKeyBeforeExit(void)
{
    fflush(stdin);
    getchar();
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        exit(-1);
    }
    filepath = std::string(argv[1]);
    std::cout << filepath << std::endl;
    atexit(WaitForEnterKeyBeforeExit); // std::atexit() is declared in cstdlib
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, 0.0, 0.0);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    if (!GLAD_GL_VERSION_4_5) {
        std::cout << "OpenGL not version 4.5" << std::endl;
        exit(-1);
    }

    Renderer renderer(window, SCR_HEIGHT, SCR_WIDTH);
    renderer.loadScene(filepath);
    renderer.startRenderLoop();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}