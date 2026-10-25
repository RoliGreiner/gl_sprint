#include "sprint/gl_test.hpp"

#include <GLFW/glfw3.h>
#include <stb_image_write.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <unistd.h>

namespace sprint {

// ---------------------------------------------------------------------------
// HiddenContext
// ---------------------------------------------------------------------------
HiddenContext::HiddenContext() {
    glfwSetErrorCallback([](int code, const char* text) { std::fprintf(stderr, "GLFW error %d: %s\n", code, text); });
    if (!glfwInit()) {
        error_ = "glfwInit failed. Is a desktop session running? (The tests need a display, like your own programs.)";
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window_ = glfwCreateWindow(64, 64, "sprint tests", nullptr, nullptr);
    if (!window_) {
        error_ = "glfwCreateWindow failed for an OpenGL 3.3 core context";
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window_);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        error_ = "gladLoadGLLoader failed";
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
    }
}

HiddenContext::~HiddenContext() {
    if (window_) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

// ---------------------------------------------------------------------------
// Offscreen
// ---------------------------------------------------------------------------
Offscreen::Offscreen(int width, int height) : width_(width), height_(height) {
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    glGenTextures(1, &color_);
    glBindTexture(GL_TEXTURE_2D, color_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_, 0);

    glGenRenderbuffers(1, &depth_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("test framebuffer is incomplete (this is a problem in the test framework, not your code)");
    bind();
}

Offscreen::~Offscreen() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo_);
    glDeleteTextures(1, &color_);
    glDeleteRenderbuffers(1, &depth_);
}

void Offscreen::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);
    glClearColor(kTestClearColor.r, kTestClearColor.g, kTestClearColor.b, kTestClearColor.a);
    glClearDepth(1.0);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

glm::vec4 Offscreen::pixel(int x, int y) const {
    GLint previous = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    unsigned char rgba[4] = {0, 0, 0, 0};
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previous));
    return glm::vec4(rgba[0], rgba[1], rgba[2], rgba[3]) / 255.0f;
}

glm::ivec2 Offscreen::pixelOf(float ndcX, float ndcY) const {
    int x = static_cast<int>((ndcX * 0.5f + 0.5f) * width_);
    int y = static_cast<int>((ndcY * 0.5f + 0.5f) * height_);
    return {glm::clamp(x, 0, width_ - 1), glm::clamp(y, 0, height_ - 1)};
}

glm::vec4 Offscreen::at(float ndcX, float ndcY) const {
    const glm::ivec2 p = pixelOf(ndcX, ndcY);
    return pixel(p.x, p.y);
}

glm::vec2 Offscreen::ndcOf(int x, int y) const {
    return {(x + 0.5f) / width_ * 2.0f - 1.0f, (y + 0.5f) / height_ * 2.0f - 1.0f};
}

bool Offscreen::anyDrawnNear(int x, int y, int r) const {
    for (int dy = -r; dy <= r; ++dy)
        for (int dx = -r; dx <= r; ++dx) {
            const int px = x + dx, py = y + dy;
            if (px < 0 || py < 0 || px >= width_ || py >= height_) continue;
            const glm::vec4 c = pixel(px, py);
            if (glm::length(glm::vec3(c) - glm::vec3(kTestClearColor)) > 0.05f) return true;
        }
    return false;
}

// ---------------------------------------------------------------------------
// Programs, meshes, textures
// ---------------------------------------------------------------------------
static GLuint compileTestShader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048] = {};
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        glDeleteShader(s);
        throw std::runtime_error(std::string("test shader failed to compile: ") + log);
    }
    return s;
}

GLuint buildTestProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vs = compileTestShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compileTestShader(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048] = {};
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        glDeleteProgram(p);
        throw std::runtime_error(std::string("test program failed to link: ") + log);
    }
    return p;
}

TestMesh::TestMesh(const std::vector<float>& data, std::initializer_list<int> attributeSizes) {
    int floatsPerVertex = 0;
    for (int s : attributeSizes) floatsPerVertex += s;
    count_ = static_cast<GLsizei>(data.size() / floatsPerVertex);

    GLint previousVao = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVao);
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), data.data(), GL_STATIC_DRAW);
    int location = 0, offset = 0;
    for (int s : attributeSizes) {
        glVertexAttribPointer(location, s, GL_FLOAT, GL_FALSE, floatsPerVertex * static_cast<GLsizei>(sizeof(float)),
                              reinterpret_cast<void*>(static_cast<std::uintptr_t>(offset * sizeof(float))));
        glEnableVertexAttribArray(location);
        ++location;
        offset += s;
    }
    glBindVertexArray(static_cast<GLuint>(previousVao));
}

TestMesh::~TestMesh() {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void TestMesh::draw() const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, count_);
    glBindVertexArray(0);
}

std::vector<float> fullscreenTriangle(float z) { return {-1.0f, -1.0f, z, 3.0f, -1.0f, z, -1.0f, 3.0f, z}; }

GLuint makeTexture(int width, int height, const std::vector<glm::vec4>& pixels, GLint filter, GLint wrap) {
    std::vector<unsigned char> bytes;
    bytes.reserve(pixels.size() * 4);
    for (const auto& p : pixels)
        for (int c = 0; c < 4; ++c) bytes.push_back(static_cast<unsigned char>(glm::clamp(p[c], 0.0f, 1.0f) * 255.0f + 0.5f));
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

GLuint makeSolidTexture(const glm::vec4& color) { return makeTexture(1, 1, {color}); }

// ---------------------------------------------------------------------------
// Uniform read-back
// ---------------------------------------------------------------------------
static GLint locationOrThrow(GLuint program, const char* name) {
    const GLint loc = glGetUniformLocation(program, name);
    if (loc < 0)
        throw std::runtime_error(std::string("uniform \"") + name +
                                 "\" not found in the program (misspelled, or unused so the compiler removed it)");
    return loc;
}

bool hasUniform(GLuint program, const char* name) { return glGetUniformLocation(program, name) >= 0; }

float uniformFloat(GLuint program, const char* name) {
    float v = 0;
    glGetUniformfv(program, locationOrThrow(program, name), &v);
    return v;
}
int uniformInt(GLuint program, const char* name) {
    GLint v = 0;
    glGetUniformiv(program, locationOrThrow(program, name), &v);
    return v;
}
glm::vec3 uniformVec3(GLuint program, const char* name) {
    glm::vec3 v(0.0f);
    glGetUniformfv(program, locationOrThrow(program, name), &v[0]);
    return v;
}
glm::mat3 uniformMat3(GLuint program, const char* name) {
    glm::mat3 m(0.0f);
    glGetUniformfv(program, locationOrThrow(program, name), &m[0][0]);
    return m;
}
glm::mat4 uniformMat4(GLuint program, const char* name) {
    glm::mat4 m(0.0f);
    glGetUniformfv(program, locationOrThrow(program, name), &m[0][0]);
    return m;
}

// ---------------------------------------------------------------------------
// Files
// ---------------------------------------------------------------------------
std::string tempDir() {
    static std::atomic<int> counter{0};
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() /
                   ("gl-sprint-test-" + std::to_string(getpid()) + "-" + std::to_string(counter++));
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

void writeFile(const std::string& path, const std::string& contents) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("could not write " + path);
    f << contents;
}

void writePng(const std::string& path, int width, int height, int channels,
              const std::vector<unsigned char>& bytesTopRowFirst) {
    if (!stbi_write_png(path.c_str(), width, height, channels, bytesTopRowFirst.data(), width * channels))
        throw std::runtime_error("could not write test image " + path);
}

const char* glErrorName(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "unknown GL error";
    }
}

}  // namespace sprint
