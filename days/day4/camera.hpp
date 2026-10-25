#pragma once
// Day 4: a fly camera (lessons 4.1 to 4.3).

#include <glm/glm.hpp>

namespace day4 {

enum class Move { Forward, Backward, Left, Right, Up, Down };

class Camera {
public:
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    float yaw = -90.0f;        // degrees. -90 looks down -z
    float pitch = 0.0f;        // degrees, always kept in [-89, 89]
    float fov = 45.0f;         // degrees, always kept in [1, 45]
    float speed = 2.5f;        // world units per second
    float sensitivity = 0.1f;  // degrees per pixel of mouse movement

    // Unit vector the camera looks along, from yaw and pitch.
    glm::vec3 front() const;
    // Unit vector to the camera's right: perpendicular to front() and the world up (0, 1, 0).
    glm::vec3 right() const;
    // glm::lookAt(position, position + front(), world up).
    glm::mat4 view() const;

    // Forward/Backward move along front(), Left/Right along right(), Up/Down along
    // the world up (0, 1, 0). Distance = speed * deltaSeconds.
    void move(Move direction, float deltaSeconds);

    // Mouse movement in pixels as GLFW reports it (y grows downwards).
    // Moving right turns right (yaw grows). Moving up looks up (pitch grows).
    void look(float dxPixels, float dyPixels);

    // Scroll wheel: positive scrollY zooms in, fov gets smaller by scrollY degrees.
    void zoom(float scrollY);
};

// Turns absolute cursor positions into movement since the previous call.
// The very first call returns (0, 0), so the camera doesn't jump at startup.
class MouseTracker {
public:
    glm::vec2 delta(double x, double y);

private:
    bool first_ = true;
    double lastX_ = 0.0;
    double lastY_ = 0.0;
};

}  // namespace day4
