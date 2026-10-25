#pragma once
// Day 3: matrices and the depth buffer (lessons 3.1 to 3.5).

#include <glm/glm.hpp>

namespace day3 {

// Model matrix that applies, to a vertex: first `scale`, then a rotation of
// `angleRadians` around `axis`, then a move to `position`.
glm::mat4 makeModel(const glm::vec3& position, float angleRadians, const glm::vec3& axis, const glm::vec3& scale);

// Projection matrix for a framebuffer of fbWidth x fbHeight pixels.
//  perspective = true:  vertical field of view of fovyDegrees.
//  perspective = false: orthographic, showing orthoHalfHeight world units above and
//                       below the center, and as much width as the aspect ratio gives.
// A framebuffer height of 0 (minimized window) must still give a finite matrix:
// use an aspect ratio of 1 then.
glm::mat4 makeProjection(bool perspective, float fovyDegrees, int fbWidth, int fbHeight, float nearPlane,
                         float farPlane, float orthoHalfHeight = 3.0f);

// Rotation angle in radians of cube number `index` (0 to 9) at `timeSeconds`.
// It must be proportional to time (0 at t = 0, twice as big at twice the time),
// and each of the 10 cubes must spin at a different, non-zero speed.
float cubeAngle(int index, double timeSeconds);

// Start of a frame: clear the color to (0.1, 0.12, 0.16, 1.0), clear the depth
// buffer, and enable or disable the depth test.
void beginFrame(bool depthTest);

}  // namespace day3
