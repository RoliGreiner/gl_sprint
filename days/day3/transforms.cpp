// Day 3: matrices and the depth buffer. Replace the TODOs, then run day3_tests.
#include "day3/transforms.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace day3 {

glm::mat4 makeModel([[maybe_unused]] const glm::vec3& position, [[maybe_unused]] float angleRadians,
                    [[maybe_unused]] const glm::vec3& axis, [[maybe_unused]] const glm::vec3& scale) {
    // TODO: lesson 3.2. Start from glm::mat4(1.0f).
    return glm::mat4(1.0f);
}

glm::mat4 makeProjection([[maybe_unused]] bool perspective, [[maybe_unused]] float fovyDegrees,
                         [[maybe_unused]] int fbWidth, [[maybe_unused]] int fbHeight, [[maybe_unused]] float nearPlane,
                         [[maybe_unused]] float farPlane, [[maybe_unused]] float orthoHalfHeight) {
    // TODO: glm::perspective or glm::ortho (lesson 3.4)
    return glm::mat4(1.0f);
}

float cubeAngle([[maybe_unused]] int index, [[maybe_unused]] double timeSeconds) {
    // TODO
    return 0.0f;
}

void beginFrame([[maybe_unused]] bool depthTest) {
    // TODO: lesson 3.5
}

}  // namespace day3
