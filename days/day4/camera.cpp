// Day 4: the fly camera. Replace the TODOs, then run day4_tests.
#include "day4/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace day4 {

glm::vec3 Camera::front() const {
    // TODO: lesson 4.2
    return {0.0f, 0.0f, 0.0f};
}

glm::vec3 Camera::right() const {
    // TODO: a cross product (lesson 4.1)
    return {0.0f, 0.0f, 0.0f};
}

glm::mat4 Camera::view() const {
    // TODO: glm::lookAt
    return glm::mat4(1.0f);
}

void Camera::move([[maybe_unused]] Move direction, [[maybe_unused]] float deltaSeconds) {
    // TODO
}

void Camera::look([[maybe_unused]] float dxPixels, [[maybe_unused]] float dyPixels) {
    // TODO: and keep pitch in [-89, 89]
}

void Camera::zoom([[maybe_unused]] float scrollY) {
    // TODO: and keep fov in [1, 45]
}

glm::vec2 MouseTracker::delta([[maybe_unused]] double x, [[maybe_unused]] double y) {
    // TODO: the first call only remembers the position (lesson 4.2)
    (void)first_;
    (void)lastX_;
    (void)lastY_;
    return {0.0f, 0.0f};
}

}  // namespace day4
