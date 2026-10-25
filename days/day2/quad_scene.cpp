// Day 2: the textured quad. Replace the TODOs, then run day2_tests.
#include "day2/quad_scene.hpp"

#include "sprint/window.hpp"  // SPRINT_DAY_FILE

namespace day2 {

QuadScene createQuadScene() {
    QuadScene scene;
    scene.shader = std::make_unique<Shader>(SPRINT_DAY_FILE("shaders/quad.vert"), SPRINT_DAY_FILE("shaders/quad.frag"));
    // TODO: 4 vertices with position + texture coordinate (lesson 2.4), 6 indices, VAO/VBO/EBO.
    return scene;
}

void drawQuadScene([[maybe_unused]] const QuadScene& scene, [[maybe_unused]] GLuint tex0, [[maybe_unused]] GLuint tex1,
                   [[maybe_unused]] float mixAmount, [[maybe_unused]] float uvScale) {
    // TODO: use the shader, set the sampler uniforms to units 0 and 1, bind the textures
    // to those units (lesson 2.9), set mixAmount and uvScale, draw.
}

void destroyQuadScene([[maybe_unused]] QuadScene& scene) {
    // TODO
}

float updateMix(float current, [[maybe_unused]] bool upHeld, [[maybe_unused]] bool downHeld,
                [[maybe_unused]] float deltaSeconds) {
    // TODO: lesson 2.10
    return current;
}

}  // namespace day2
