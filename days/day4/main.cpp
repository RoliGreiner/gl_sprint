// Day 4 program: fly around a lit cube.
//   WASD + Space/Ctrl: move    mouse: look    scroll: zoom    B: Phong / Blinn-Phong    Esc: quit
#include "day3/cube.hpp"
#include "day4/camera.hpp"
#include "day4/lighting.hpp"

#include "sprint/window.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>

namespace {

struct App {
    day4::Camera camera;
    day4::MouseTracker mouse;
    bool blinn = false;
};

App& app(GLFWwindow* w) { return *static_cast<App*>(glfwGetWindowUserPointer(w)); }

}  // namespace

int main() {
    GLFWwindow* window = sprint::openWindow("gl-sprint day 4");
    if (!window) return 1;

    App state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        const glm::vec2 d = app(w).mouse.delta(x, y);
        app(w).camera.look(d.x, d.y);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* w, double, double dy) { app(w).camera.zoom(static_cast<float>(dy)); });
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_B && action == GLFW_PRESS) app(w).blinn = !app(w).blinn;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    });

    auto lit = day4::makeLitShader();
    auto lamp = day4::makeLightCubeShader();
    if (!lit->valid() || !lamp->valid()) {
        std::fprintf(stderr, "%s\n%s\n", lit->error().c_str(), lamp->error().c_str());
        return 1;
    }
    day3::Cube lampCube = day3::makeCube();  // positions are at location 0, which is all the lamp shader needs

    // TODO: a cube with positions (location 0) and normals (location 1), 6 floats per vertex.
    // LearnOpenGL's Basic Lighting chapter has the vertex list.
    GLuint litVao = 0;

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        const float dt = sprint::frameDelta();
        day4::Camera& cam = state.camera;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.move(day4::Move::Forward, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.move(day4::Move::Backward, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam.move(day4::Move::Left, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam.move(day4::Move::Right, dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cam.move(day4::Move::Up, dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cam.move(day4::Move::Down, dt);

        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        const float aspect = fbh > 0 ? static_cast<float>(fbw) / static_cast<float>(fbh) : 1.0f;
        const glm::mat4 projection = glm::perspective(glm::radians(cam.fov), aspect, 0.1f, 100.0f);
        const glm::mat4 view = cam.view();
        const float t = static_cast<float>(glfwGetTime());
        const glm::vec3 lightPos(1.5f * std::cos(t), 1.0f, 1.5f * std::sin(t));

        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: draw the lit cube with the lit shader: set every uniform listed in lighting.hpp.
        (void)litVao;

        const glm::mat4 lampModel = glm::scale(glm::translate(glm::mat4(1.0f), lightPos), glm::vec3(0.2f));
        lamp->use();
        lamp->setMat4("model", lampModel);
        lamp->setMat4("view", view);
        lamp->setMat4("projection", projection);
        glBindVertexArray(lampCube.vao);
        glDrawArrays(GL_TRIANGLES, 0, lampCube.count);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    day3::destroyCube(lampCube);
    glfwTerminate();
}
