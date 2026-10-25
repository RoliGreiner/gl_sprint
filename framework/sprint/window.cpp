#include "sprint/window.hpp"

#include <cstdio>

namespace sprint {

GLFWwindow* openWindow(const char* title, int width, int height) {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Could not create a window with OpenGL 3.3 core\n");
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::fprintf(stderr, "gladLoadGLLoader failed\n");
        glfwDestroyWindow(window);
        return nullptr;
    }
    glfwSwapInterval(1);

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); });
    return window;
}

float frameDelta() {
    static double last = -1.0;
    const double now = glfwGetTime();
    const double dt = last < 0.0 ? 0.0 : now - last;
    last = now;
    return static_cast<float>(dt);
}

}  // namespace sprint
