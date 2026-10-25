#pragma once
// Day 4: normals and Phong lighting (lessons 4.4 to 4.6).

#include "day2/shader.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace day4 {

// The matrix that moves normals into world space: transpose(inverse(mat3(model))).
glm::mat3 normalMatrix(const glm::mat4& model);

// Loads SPRINT_DAY_FILE("shaders/lit.vert") and ("shaders/lit.frag").
//
// Vertex inputs:  location 0 vec3 position, location 1 vec3 normal.
// Uniforms:       mat4 model, view, projection; mat3 normalMatrix;
//                 vec3 lightPos, viewPos, lightColor, objectColor; bool blinn.
// Lighting is done in world space, with N, L and V normalized in the fragment shader:
//   ambient  = 0.1 * lightColor
//   diffuse  = max(dot(N, L), 0) * lightColor
//   Phong:       specular = 0.5 * pow(max(dot(V, reflect(-L, N)), 0), 32) * lightColor
//   Blinn-Phong: specular = 0.5 * pow(max(dot(N, normalize(L + V)), 0), 32) * lightColor
//   color    = (ambient + diffuse + specular) * objectColor, alpha 1
// The tests compare your pixels with exactly these formulas.
std::unique_ptr<day2::Shader> makeLitShader();

// Loads SPRINT_DAY_FILE("shaders/light.vert") and ("shaders/light.frag"):
// position at location 0, mat4 model, view, projection, and the color is always white.
std::unique_ptr<day2::Shader> makeLightCubeShader();

}  // namespace day4
