#pragma once
// Day 2: the textured quad you control with the arrow keys.

#include "day2/shader.hpp"

#include <glad/glad.h>

#include <memory>

namespace day2 {

struct QuadScene {
    std::unique_ptr<Shader> shader;  // from shaders/quad.vert and shaders/quad.frag in this folder
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
};

// Builds the quad from (-0.5, -0.5) to (0.5, 0.5) at z = 0 with texture coordinates
// from (0, 0) at the bottom-left corner to (1, 1) at the top-right, using 4 vertices
// and 6 indices. Loads the shader with SPRINT_DAY_FILE("shaders/quad.vert") and
// SPRINT_DAY_FILE("shaders/quad.frag") (from sprint/window.hpp), so it works from any
// working directory. Writing those two shader files is part of the exercise.
QuadScene createQuadScene();

// Draws the quad with tex0 on texture unit 0 and tex1 on texture unit 1.
// Fragment color = mix(texture(tex0, uv * uvScale), texture(tex1, uv * uvScale), mixAmount).
void drawQuadScene(const QuadScene& scene, GLuint tex0, GLuint tex1, float mixAmount, float uvScale = 1.0f);

void destroyQuadScene(QuadScene& scene);

// New mix amount after one frame. Holding Up raises it by 1.0 per second, holding
// Down lowers it by 1.0 per second, both or neither leave it unchanged.
// The result is always kept between 0 and 1.
float updateMix(float current, bool upHeld, bool downHeld, float deltaSeconds);

}  // namespace day2
