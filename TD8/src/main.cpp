#include "opengl.h"
#include "viewer.h"

Viewer* v;

int WIDTH = 640;
int HEIGHT = 480;

static void char_callback(GLFWwindow* window, unsigned int key)
{
    v->charPressed(key);
}

static void scroll_callback(GLFWwindow* window, double x, double y)
{
    v->mouseScroll(x,y);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    v->keyPressed(key,action,mods);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    v->mousePressed(window, button, action, mods);
}

void cursorPos_callback(GLFWwindow* window, double x, double y)
{
    v->mouseMoved(x,y);
}

void  reshape_callback(GLFWwindow* window, int width, int height)
{
    v->reshape(width,height);
}

// initialize GLFW framework
GLFWwindow* initGLFW()
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, (GLint)GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "SIA TD10", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glbinding::Binding::initialize();
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetFramebufferSizeCallback(window, reshape_callback);
    glfwSetCursorPosCallback(window, cursorPos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}


int main (int argc, char **argv)
{
    glfwSetErrorCallback(error_callback);

    GLFWwindow* window = initGLFW();
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    v = new Viewer();
    v->init(w,h);

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window))
    {
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            glfwSetWindowTitle(window, (std::string("SIA TD8 - ") + std::to_string(1000.0/double(nbFrames)) + std::string(" ms/frame")).c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }
        // render the scene
        v->updateScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete v;

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
