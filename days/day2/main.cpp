// Day 2 program: a quad mixing two textures, Up/Down change the mix.
// Put two images in days/day2/assets/, for example container.jpg and awesomeface.png
// from LearnOpenGL's Textures chapter.
#include "day2/quad_scene.hpp"
#include "day2/texture.hpp"

#include "sprint/window.hpp"

#include <cstdio>

int main() {
    GLFWwindow* window = sprint::openWindow("gl-sprint day 2");
    if (!window) return 1;

    day2::QuadScene scene = day2::createQuadScene();
    if (!scene.shader || !scene.shader->valid()) {
        std::fprintf(stderr, "%s\n", scene.shader ? scene.shader->error().c_str() : "no shader");
        return 1;
    }
    GLuint tex0 = day2::loadTexture(SPRINT_DAY_FILE("assets/container.jpg"));
    GLuint tex1 = day2::loadTexture(SPRINT_DAY_FILE("assets/awesomeface.png"));
    if (!tex0 || !tex1) std::fprintf(stderr, "Could not load the images in days/day2/assets/\n");

    float mixAmount = 0.2f;
    float uvScale = 1.0f;  // TODO: a key that switches this to 2 to check GL_REPEAT tiling
    while (!glfwWindowShouldClose(window)) {
        const float dt = sprint::frameDelta();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        mixAmount = day2::updateMix(mixAmount, glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS,
                                    glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS, dt);

        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        day2::drawQuadScene(scene, tex0, tex1, mixAmount, uvScale);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    day2::destroyQuadScene(scene);
    glDeleteTextures(1, &tex0);
    glDeleteTextures(1, &tex1);
    glfwTerminate();
}
