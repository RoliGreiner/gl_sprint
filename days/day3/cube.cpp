// Day 3: the textured cube. Replace the TODOs, then run day3_tests.
#include "day3/cube.hpp"

#include "sprint/window.hpp"  // SPRINT_DAY_FILE

namespace day3 {

Cube makeCube() {
    // TODO: 36 vertices x 5 floats (lesson 3.6). You may take the vertex list from
    // LearnOpenGL's Coordinate Systems chapter, but write the buffer setup yourself.
    return {};
}

void destroyCube([[maybe_unused]] Cube& cube) {
    // TODO
}

std::unique_ptr<day2::Shader> makeCubeShader() {
    return std::make_unique<day2::Shader>(SPRINT_DAY_FILE("shaders/cube.vert"), SPRINT_DAY_FILE("shaders/cube.frag"));
}

void drawCube([[maybe_unused]] const Cube& cube, [[maybe_unused]] const day2::Shader& shader,
              [[maybe_unused]] const glm::mat4& model, [[maybe_unused]] const glm::mat4& view,
              [[maybe_unused]] const glm::mat4& projection, [[maybe_unused]] GLuint texture) {
    // TODO: use the shader, set the three matrices with setMat4, bind the texture to unit 0, draw.
}

}  // namespace day3
