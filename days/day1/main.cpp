// Day 1 program: the window code from the plan, plus TODOs for your drawing.
// Run it with the "day1" target. Automatic checks are in the "day1_tests" target.
//
// What it should do when you are done:
//   - draw the left triangle orange and the right triangle yellow
//   - key 2: switch between the two triangles and the rectangle
//   - key W: toggle wireframe
//   - Esc: quit
#include "day1/exercise.hpp"

#include <GLFW/glfw3.h>

#include <cstdio>

static void on_resize(GLFWwindow*, int w, int h) { day1::onFramebufferResize(w, h); }

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "gl-sprint day 1", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "window creation failed\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::fprintf(stderr, "glad failed\n");
        return 1;
    }

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    day1::onFramebufferResize(fbw, fbh);
    glfwSetFramebufferSizeCallback(window, on_resize);
    std::printf("%s | %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    // TODO: build the two programs and the three meshes here. Print the log and exit if a program fails.

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        // TODO: keys 2 and W. Toggle only once per press: remember whether the key was down last frame,
        // or use glfwSetKeyCallback (lesson 4.3 explains why).

        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: draw

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // TODO: destroy meshes and programs
    glfwTerminate();
}
