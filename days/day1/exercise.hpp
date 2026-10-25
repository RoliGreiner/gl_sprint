#pragma once
// Day 1: Two triangles, one rectangle.
//
// Implement every function below in exercise.cpp. The tests in tests/day1_tests.cpp call
// them and check the result on the GPU, so keep the names and parameters as they are.
// Then use them in main.cpp to build the interactive program.

#include <glad/glad.h>

#include <string>

namespace day1 {

// --- Shaders (lesson 1.7) ----------------------------------------------------

// Compiles one shader stage from source.
// Success: returns the new shader object.
// Failure: deletes the shader object, writes the driver's info log into *log
//          (when log is not nullptr) and returns 0.
GLuint compileShader(GLenum type, const char* source, std::string* log);

// Links a vertex and a fragment shader into a program.
// Success: returns the program. Failure: deletes the program, writes the info log
// into *log (when not nullptr) and returns 0. It does not delete the shaders.
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader, std::string* log);

// Compiles both sources and links them. The shader objects are deleted afterwards
// in every case, because the program keeps what it needs. Returns 0 on any failure
// and puts the reason into *log.
GLuint buildProgram(const char* vertexSource, const char* fragmentSource, std::string* log);

// Both programs read the vertex position from attribute location 0 and pass it on
// unchanged. The orange one outputs (1.0, 0.5, 0.2, 1.0), the yellow one (1.0, 1.0, 0.0, 1.0).
GLuint makeOrangeProgram(std::string* log);
GLuint makeYellowProgram(std::string* log);

// --- Meshes (lessons 1.5, 1.6, 1.8) -------------------------------------------

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;        // 0 when not indexed
    GLsizei count = 0;     // vertices for glDrawArrays, indices for glDrawElements
    bool indexed = false;
};

// Each mesh has its own VAO and VBO. Vertices hold only a position: 3 floats
// (x, y, z = 0) at attribute location 0.
//
// Left triangle:  (-0.9, -0.5), (-0.1, -0.5), (-0.5, 0.5)
// Right triangle: ( 0.1, -0.5), ( 0.9, -0.5), ( 0.5, 0.5)
Mesh makeLeftTriangle();
Mesh makeRightTriangle();

// The square from (-0.5, -0.5) to (0.5, 0.5): exactly 4 vertices in the VBO and
// 6 indices (unsigned int) in an EBO, giving two triangles.
Mesh makeRectangle();

// Binds the mesh's VAO and draws it with glDrawElements or glDrawArrays.
void drawMesh(const Mesh& mesh);

// Deletes the mesh's VAO and buffers and resets it to an empty Mesh.
void destroyMesh(Mesh& mesh);

// --- State (lessons 1.2, 1.3) ---------------------------------------------------

// true: draw triangle outlines only. false: filled triangles again.
void setWireframe(bool enabled);

// What your framebuffer-size callback does: make the viewport cover the
// whole framebuffer of the new size.
void onFramebufferResize(int width, int height);

}  // namespace day1
