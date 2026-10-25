#pragma once
// OpenGL helpers used by the tests. Provided, don't edit.
// Tests render into an offscreen framebuffer and read pixels back, so nothing
// appears on screen and the result does not depend on your window size.

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <initializer_list>
#include <string>
#include <vector>

struct GLFWwindow;

namespace sprint {

// The clear color the tests use for "nothing drawn here".
inline const glm::vec4 kTestClearColor{0.0f, 0.0f, 0.0f, 1.0f};
// Colors read back from an 8-bit framebuffer are only accurate to about 1/255.
inline constexpr float kPixelEps = 3.0f / 255.0f;

// Hidden window with an OpenGL 3.3 core context, created once by the test runner.
class HiddenContext {
public:
    HiddenContext();
    ~HiddenContext();
    bool ok() const { return window_ != nullptr; }
    const std::string& error() const { return error_; }

private:
    GLFWwindow* window_ = nullptr;
    std::string error_;
};

// An offscreen RGBA8 color buffer with a depth buffer.
class Offscreen {
public:
    explicit Offscreen(int width = 64, int height = 64);
    ~Offscreen();
    Offscreen(const Offscreen&) = delete;
    Offscreen& operator=(const Offscreen&) = delete;

    // Binds it, sets the viewport to cover it, clears color to kTestClearColor and depth to 1.
    void bind();
    // Color of one pixel, (0,0) is bottom-left, values 0..1.
    glm::vec4 pixel(int x, int y) const;
    // Color of the pixel that contains this normalized device coordinate.
    glm::vec4 at(float ndcX, float ndcY) const;
    // Pixel coordinate that contains this NDC point.
    glm::ivec2 pixelOf(float ndcX, float ndcY) const;
    // NDC of the center of a pixel.
    glm::vec2 ndcOf(int x, int y) const;
    // True if any pixel in the (2r+1)x(2r+1) block around (x, y) differs from the clear color.
    bool anyDrawnNear(int x, int y, int r = 1) const;

    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_, height_;
    GLuint fbo_ = 0, color_ = 0, depth_ = 0;
};

// A program built from source by the tests themselves, independent of your code.
// Throws std::runtime_error with the info log if it fails.
GLuint buildTestProgram(const char* vertexSource, const char* fragmentSource);

// Interleaved float vertex data with attributes at locations 0, 1, 2, ...
class TestMesh {
public:
    TestMesh(const std::vector<float>& data, std::initializer_list<int> attributeSizes);
    ~TestMesh();
    TestMesh(const TestMesh&) = delete;
    TestMesh& operator=(const TestMesh&) = delete;
    void draw() const;

private:
    GLuint vao_ = 0, vbo_ = 0;
    GLsizei count_ = 0;
};

// Positions only: one triangle that covers the whole screen, at depth z.
std::vector<float> fullscreenTriangle(float z = 0.0f);

// A texture from RGBA8 pixels, first row = bottom of the image (OpenGL order).
GLuint makeTexture(int width, int height, const std::vector<glm::vec4>& pixelsBottomRowFirst,
                   GLint filter = GL_NEAREST, GLint wrap = GL_REPEAT);
GLuint makeSolidTexture(const glm::vec4& color);

// Value of a uniform in a program, read back from the driver.
float uniformFloat(GLuint program, const char* name);
int uniformInt(GLuint program, const char* name);
glm::vec3 uniformVec3(GLuint program, const char* name);
glm::mat3 uniformMat3(GLuint program, const char* name);
glm::mat4 uniformMat4(GLuint program, const char* name);
bool hasUniform(GLuint program, const char* name);

// A fresh empty folder for files written by a test.
std::string tempDir();
void writeFile(const std::string& path, const std::string& contents);
// Writes a PNG. Rows are given top row first, like an image file stores them.
void writePng(const std::string& path, int width, int height, int channels,
              const std::vector<unsigned char>& bytesTopRowFirst);

const char* glErrorName(GLenum error);

}  // namespace sprint
