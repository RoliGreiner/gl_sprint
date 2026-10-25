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

    std::string log;
    GLuint orange_program = day1::makeOrangeProgram(&log);
    GLuint yellow_program = day1::makeYellowProgram(&log);

    if (!orange_program || !yellow_program) {
        std::fprintf(stderr, "%s\n", log.c_str());
        glfwTerminate();
        return 1;
    }

    day1::Mesh left_triangle = day1::makeLeftTriangle();
    day1::Mesh right_triangle = day1::makeRightTriangle();
    day1::Mesh rectangle = day1::makeRectangle();

    bool key_two_pressed = false;
    bool key_w_pressed = false;
    bool show_rectangle = false;
    bool wireframe = false;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        // TODO: keys 2 and W. Toggle only once per press: remember whether the key was down last frame,
        // or use glfwSetKeyCallback (lesson 4.3 explains why).
        bool two_down = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool w_down = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

        if (two_down && !key_two_pressed) {
            show_rectangle = !show_rectangle;
        }
        key_two_pressed = two_down;

        if (w_down && !key_w_pressed) {
            wireframe = !wireframe;
            day1::setWireframe(wireframe);
        }
        key_w_pressed = w_down;

        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: draw
        if (show_rectangle) {
            glUseProgram(orange_program);
            day1::drawMesh(rectangle);
        } else {
            glUseProgram(orange_program);
            day1::drawMesh(left_triangle);
            glUseProgram(yellow_program);
            day1::drawMesh(right_triangle);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // TODO: destroy meshes and programs
    day1::destroyMesh(left_triangle);
    day1::destroyMesh(right_triangle);
    day1::destroyMesh(rectangle);
    glDeleteProgram(orange_program);
    glDeleteProgram(yellow_program);
    glfwTerminate();
}
