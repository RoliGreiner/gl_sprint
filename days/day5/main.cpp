// Day 5 program: the lit crate yard.
//   WASD + Space/Ctrl: move    mouse: look    F: flashlight on/off    Esc: quit
// Put container2.png and container2_specular.png from LearnOpenGL's Lighting maps
// chapter in days/day5/assets/.
#include "day2/texture.hpp"
#include "day3/transforms.hpp"
#include "day4/camera.hpp"
#include "day4/lighting.hpp"
#include "day5/lights.hpp"

#include "sprint/window.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>

namespace {

struct App {
    day4::Camera camera;
    day4::MouseTracker mouse;
    bool flashlight = true;
};

App& app(GLFWwindow* w) { return *static_cast<App*>(glfwGetWindowUserPointer(w)); }

}  // namespace

int main() {
    GLFWwindow* window = sprint::openWindow("gl-sprint day 5");
    if (!window) return 1;

    App state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        const glm::vec2 d = app(w).mouse.delta(x, y);
        app(w).camera.look(d.x, d.y);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_F && action == GLFW_PRESS) app(w).flashlight = !app(w).flashlight;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    });

    auto scene = day5::makeSceneShader();
    if (!scene->valid()) {
        std::fprintf(stderr, "%s\n", scene->error().c_str());
        return 1;
    }
    day5::Material crate;
    crate.diffuseMap = day2::loadTexture(SPRINT_DAY_FILE("assets/container2.png"));
    crate.specularMap = day2::loadTexture(SPRINT_DAY_FILE("assets/container2_specular.png"));

    // TODO: a cube VAO with position (location 0), normal (1) and texture coordinate (2),
    // 8 floats per vertex. LearnOpenGL's Lighting maps chapter has the vertex list.
    GLuint crateVao = 0;

    day5::DirLight sun;
    day5::PointLight lamps[day5::kMaxPointLights];
    // TODO: give the four lamps positions and colors.
    day5::SpotLight flashlight;

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
        const glm::mat4 projection = day3::makeProjection(true, cam.fov, fbw, fbh, 0.1f, 100.0f);

        day3::beginFrame(true);
        scene->use();
        scene->setMat4("view", cam.view());
        scene->setMat4("projection", projection);
        scene->setVec3("viewPos", cam.position);
        day5::applyMaterial(*scene, crate);
        day5::uploadDirLight(*scene, sun);
        for (int i = 0; i < day5::kMaxPointLights; ++i) day5::uploadPointLight(*scene, lamps[i], i);
        day5::uploadPointLightCount(*scene, day5::kMaxPointLights);
        flashlight.position = cam.position;
        flashlight.direction = cam.front();
        day5::uploadSpotLight(*scene, flashlight, state.flashlight);

        // TODO: draw ten crates (model and normalMatrix per crate), and a small light cube
        // at each lamp with the Day 4 light shader.
        (void)crateVao;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
