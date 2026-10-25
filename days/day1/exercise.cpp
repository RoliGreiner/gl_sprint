// Day 1 exercise. Replace each TODO with your code, then run day1_tests.
// The comments in exercise.hpp say exactly what each function must do.
#include "day1/exercise.hpp"

namespace day1 {

GLuint compileShader([[maybe_unused]] GLenum type, [[maybe_unused]] const char* source,
                     [[maybe_unused]] std::string* log) {
    // TODO: glCreateShader, glShaderSource, glCompileShader, then check GL_COMPILE_STATUS.
    return 0;
}

GLuint linkProgram([[maybe_unused]] GLuint vertexShader, [[maybe_unused]] GLuint fragmentShader,
                   [[maybe_unused]] std::string* log) {
    // TODO: glCreateProgram, attach both shaders, glLinkProgram, then check GL_LINK_STATUS.
    return 0;
}

GLuint buildProgram([[maybe_unused]] const char* vertexSource, [[maybe_unused]] const char* fragmentSource,
                    [[maybe_unused]] std::string* log) {
    // TODO: use compileShader and linkProgram, and delete the shader objects afterwards.
    return 0;
}

GLuint makeOrangeProgram([[maybe_unused]] std::string* log) {
    // TODO: write the vertex and fragment shader sources and call buildProgram.
    return 0;
}

GLuint makeYellowProgram([[maybe_unused]] std::string* log) {
    // TODO
    return 0;
}

Mesh makeLeftTriangle() {
    // TODO: VAO + VBO, glBufferData, glVertexAttribPointer for location 0, glEnableVertexAttribArray.
    return {};
}

Mesh makeRightTriangle() {
    // TODO
    return {};
}

Mesh makeRectangle() {
    // TODO: 4 vertices in the VBO, 6 indices in an EBO. Mind lesson 1.8 about the EBO and the VAO.
    return {};
}

void drawMesh([[maybe_unused]] const Mesh& mesh) {
    // TODO
}

void destroyMesh([[maybe_unused]] Mesh& mesh) {
    // TODO
}

void setWireframe([[maybe_unused]] bool enabled) {
    // TODO: glPolygonMode
}

void onFramebufferResize([[maybe_unused]] int width, [[maybe_unused]] int height) {
    // TODO
}

}  // namespace day1
