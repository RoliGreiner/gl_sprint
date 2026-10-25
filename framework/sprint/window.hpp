#pragma once
// Provided helper for the Day 2-5 programs, so each day can focus on its topic.
// It does what you wrote yourself on Day 1: GLFW init, a 3.3 core window, glad, viewport.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

namespace sprint {

// Opens a window with an OpenGL 3.3 core context, loads glad, turns on vsync and
// keeps the viewport matched to the framebuffer when the window is resized.
// Returns nullptr (and prints why) on failure. Call glfwTerminate() when done.
GLFWwindow* openWindow(const char* title, int width = 1280, int height = 720);

// Seconds since the previous call. The first call returns 0.
float frameDelta();

}  // namespace sprint

// Absolute path of a file inside the current day's folder, e.g.
// SPRINT_DAY_FILE("shaders/quad.vert") -> ".../days/day2/shaders/quad.vert".
// CMake sets SPRINT_DAY_DIR for each day, so this works from any working directory.
#define SPRINT_DAY_FILE(relative) (std::string(SPRINT_DAY_DIR) + "/" + (relative))
