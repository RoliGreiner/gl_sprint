// Day 3 program: ten spinning crates.
//   D: toggle the depth test    P: switch perspective / orthographic    Esc: quit
// Uses days/day2/assets/container.jpg.
#include "day2/texture.hpp"
#include "day3/cube.hpp"
#include "day3/transforms.hpp"

#include "sprint/window.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>

int main() {
    GLFWwindow* window = sprint::openWindow("gl-sprint day 3");
    if (!window) return 1;

    auto shader = day3::makeCubeShader();
    if (!shader->valid()) {
        std::fprintf(stderr, "%s\n", shader->error().c_str());
        return 1;
    }
    day3::Cube cube = day3::makeCube();
    GLuint texture = day2::loadTexture(std::string(SPRINT_DAY_DIR) + "/../day2/assets/container.jpg");

    const glm::vec3 positions[10] = {{0.0f, 0.0f, 0.0f},    {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
                                     {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
                                     {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
                                     {-1.3f, 1.0f, -1.5f}};
    bool depthTest = true;
    bool perspective = true;

    // TODO: toggle depthTest with D and perspective with P, once per key press.

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        const glm::mat4 projection = day3::makeProjection(perspective, 45.0f, fbw, fbh, 0.1f, 100.0f);
        const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

        day3::beginFrame(depthTest);
        for (int i = 0; i < 10; ++i) {
            const glm::mat4 model = day3::makeModel(positions[i], day3::cubeAngle(i, glfwGetTime()),
                                                    glm::vec3(1.0f, 0.3f, 0.5f), glm::vec3(1.0f));
            day3::drawCube(cube, *shader, model, view, projection, texture);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    day3::destroyCube(cube);
    glDeleteTextures(1, &texture);
    glfwTerminate();
}
