#pragma once
// Day 3: a textured cube (lesson 3.6).

#include "day2/shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

namespace day3 {

struct Cube {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei count = 0;  // 36
};

// A cube from -0.5 to 0.5 on every axis, as 36 vertices (no EBO).
// Each vertex is 5 floats: position x y z at location 0, texture coordinate u v at location 1.
// Every face shows the whole texture (0..1). On the front face, the one facing +z,
// the texture is upright when seen from +z: (0, 0) at its bottom-left corner.
Cube makeCube();
void destroyCube(Cube& cube);

// Loads SPRINT_DAY_FILE("shaders/cube.vert") and SPRINT_DAY_FILE("shaders/cube.frag").
// The vertex shader uses mat4 uniforms "model", "view" and "projection".
// The fragment shader outputs the color of the texture on unit 0.
std::unique_ptr<day2::Shader> makeCubeShader();

// Draws the cube with the given matrices and texture (bound to unit 0).
// Does not clear and does not change the depth test.
void drawCube(const Cube& cube, const day2::Shader& shader, const glm::mat4& model, const glm::mat4& view,
              const glm::mat4& projection, GLuint texture);

}  // namespace day3
