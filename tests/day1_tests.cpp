// Automatic checks for Day 1. Provided, don't edit.
#include "day1/exercise.hpp"

#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <glm/glm.hpp>

#include <vector>

SPRINT_DAY("Day 1: Two triangles, one rectangle");

using namespace sprint;
using sprint::test::str;

namespace {

const char* kWhiteVS = R"(#version 330 core
layout(location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); })";
const char* kWhiteFS = R"(#version 330 core
out vec4 c;
void main() { c = vec4(1.0); })";

const char* kGoodVS = kWhiteVS;
const char* kBrokenFS = R"(#version 330 core
out vec4 c;
void main() { c = vec4(1.0) }   // missing semicolon
)";
const char* kNoMainFS = R"(#version 330 core
out vec4 c;
void notMain() { c = vec4(1.0); }   // compiles, but a program needs a main() to link
)";

const glm::vec4 kOrange{1.0f, 0.5f, 0.2f, 1.0f};
const glm::vec4 kYellow{1.0f, 1.0f, 0.0f, 1.0f};
const glm::vec4 kWhite{1.0f};

bool isColor(const glm::vec4& got, const glm::vec4& want) { return test::near(got, want, kPixelEps); }

GLint bufferSize(GLuint buffer, GLenum target) {
    GLint size = -1;
    glBindBuffer(target, buffer);
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &size);
    glBindBuffer(target, 0);
    return size;
}

struct WhiteProgram {
    GLuint id = buildTestProgram(kWhiteVS, kWhiteFS);
    ~WhiteProgram() { glDeleteProgram(id); }
};

}  // namespace

// ---------------------------------------------------------------------------
// Shaders
// ---------------------------------------------------------------------------
TEST("compileShader compiles a valid shader") {
    std::string log;
    GLuint s = day1::compileShader(GL_VERTEX_SHADER, kGoodVS, &log);
    REQUIRE_MSG(s != 0, "compileShader returned 0 for a valid vertex shader. Log: " << log);
    CHECK_MSG(glIsShader(s), "the returned value is not a shader object");
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    CHECK_MSG(ok == GL_TRUE, "the shader exists but was not compiled (call glCompileShader)");
    glDeleteShader(s);
}

TEST("compileShader returns 0 and the info log for a broken shader") {
    std::string log;
    GLuint s = day1::compileShader(GL_FRAGMENT_SHADER, kBrokenFS, &log);
    CHECK_MSG(s == 0, "a shader with a syntax error should give 0, got " << s);
    CHECK_MSG(!log.empty(), "log is empty. Read GL_INFO_LOG_LENGTH and glGetShaderInfoLog into *log");
    CHECK_MSG(log.find('\0') == std::string::npos, "log contains '\\0' characters. Trim the string to the real length");
    // log may be nullptr
    GLuint s2 = day1::compileShader(GL_FRAGMENT_SHADER, kBrokenFS, nullptr);
    CHECK_MSG(s2 == 0, "with log = nullptr a broken shader should still give 0 (and not crash)");
}

TEST("linkProgram links a valid pair") {
    std::string log;
    GLuint vs = day1::compileShader(GL_VERTEX_SHADER, kWhiteVS, &log);
    GLuint fs = day1::compileShader(GL_FRAGMENT_SHADER, kWhiteFS, &log);
    REQUIRE_MSG(vs && fs, "compileShader must work first. Log: " << log);
    GLuint p = day1::linkProgram(vs, fs, &log);
    REQUIRE_MSG(p != 0, "linkProgram returned 0 for a valid pair. Log: " << log);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    CHECK_MSG(ok == GL_TRUE, "the program was not linked (call glLinkProgram)");
    CHECK_MSG(glIsShader(vs) && glIsShader(fs), "linkProgram must not delete the shaders it is given");
    glDeleteProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

TEST("linkProgram returns 0 and the info log when linking fails") {
    std::string log;
    GLuint vs = day1::compileShader(GL_VERTEX_SHADER, kWhiteVS, &log);
    GLuint fs = day1::compileShader(GL_FRAGMENT_SHADER, kNoMainFS, &log);
    REQUIRE_MSG(vs && fs, "compileShader must work first. Log: " << log);
    log.clear();
    GLuint p = day1::linkProgram(vs, fs, &log);
    CHECK_MSG(p == 0, "the fragment shader has no main(), so linking must fail");
    CHECK_MSG(!log.empty(), "log is empty. Read the program info log with glGetProgramInfoLog");
    glDeleteShader(vs);
    glDeleteShader(fs);
}

TEST("buildProgram returns a working program and deletes the shader objects") {
    std::string log;
    GLuint p = day1::buildProgram(kWhiteVS, kWhiteFS, &log);
    REQUIRE_MSG(p != 0, "buildProgram returned 0. Log: " << log);
    GLint attached = 0;
    glGetProgramiv(p, GL_ATTACHED_SHADERS, &attached);
    GLuint shaders[4] = {};
    glGetAttachedShaders(p, 4, nullptr, shaders);
    for (int i = 0; i < attached; ++i) {
        GLint deleted = GL_FALSE;
        glGetShaderiv(shaders[i], GL_DELETE_STATUS, &deleted);
        CHECK_MSG(deleted == GL_TRUE, "shader " << shaders[i] << " is still alive. Call glDeleteShader after linking");
    }
    glDeleteProgram(p);

    log.clear();
    CHECK_MSG(day1::buildProgram(kWhiteVS, kBrokenFS, &log) == 0, "a broken fragment shader must make buildProgram return 0");
    CHECK_MSG(!log.empty(), "buildProgram must pass the compile log on");
}

TEST("orange and yellow programs output the right colors") {
    Offscreen target;
    TestMesh screen(fullscreenTriangle(), {3});
    std::string log;

    GLuint orange = day1::makeOrangeProgram(&log);
    REQUIRE_MSG(orange != 0, "makeOrangeProgram failed. Log: " << log);
    glUseProgram(orange);
    screen.draw();
    CHECK_MSG(isColor(target.at(0, 0), kOrange), "expected orange " << str(kOrange) << ", got " << str(target.at(0, 0)));

    GLuint yellow = day1::makeYellowProgram(&log);
    REQUIRE_MSG(yellow != 0, "makeYellowProgram failed. Log: " << log);
    glUseProgram(yellow);
    screen.draw();
    CHECK_MSG(isColor(target.at(0, 0), kYellow), "expected yellow " << str(kYellow) << ", got " << str(target.at(0, 0)));
    glUseProgram(0);
    glDeleteProgram(orange);
    glDeleteProgram(yellow);
}

// ---------------------------------------------------------------------------
// Meshes
// ---------------------------------------------------------------------------
TEST("each triangle has its own VAO and VBO with 3 positions") {
    day1::Mesh left = day1::makeLeftTriangle();
    day1::Mesh right = day1::makeRightTriangle();
    REQUIRE_MSG(left.vao && glIsVertexArray(left.vao), "left.vao is not a vertex array object");
    REQUIRE_MSG(right.vao && glIsVertexArray(right.vao), "right.vao is not a vertex array object");
    CHECK_MSG(left.vao != right.vao, "both triangles use the same VAO");
    CHECK_MSG(left.vbo != right.vbo && glIsBuffer(left.vbo) && glIsBuffer(right.vbo), "each triangle needs its own VBO");
    CHECK_MSG(left.count == 3 && !left.indexed, "left: count should be 3 and indexed false, got count " << left.count);
    CHECK_MSG(bufferSize(left.vbo, GL_ARRAY_BUFFER) == 9 * 4,
              "left VBO should hold 9 floats (36 bytes), has " << bufferSize(left.vbo, GL_ARRAY_BUFFER) << " bytes");

    glBindVertexArray(left.vao);
    GLint enabled = 0, size = 0, stride = -1;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
    glBindVertexArray(0);
    CHECK_MSG(enabled == GL_TRUE, "attribute 0 is not enabled in the VAO (glEnableVertexAttribArray)");
    CHECK_MSG(size == 3, "attribute 0 should have 3 components, has " << size);
    CHECK_MSG(stride == 0 || stride == 12, "attribute 0 stride should be 12 bytes (or 0 = tightly packed), is " << stride);
    day1::destroyMesh(left);
    day1::destroyMesh(right);
}

TEST("triangles are drawn in the right place") {
    Offscreen target;
    WhiteProgram white;
    day1::Mesh left = day1::makeLeftTriangle();
    day1::Mesh right = day1::makeRightTriangle();
    REQUIRE(left.vao && right.vao);

    glUseProgram(white.id);
    day1::drawMesh(left);
    CHECK_MSG(isColor(target.at(-0.5f, -0.17f), kWhite), "nothing drawn at the center of the left triangle");
    CHECK_MSG(!isColor(target.at(0.5f, -0.17f), kWhite), "drawing the left triangle also drew where the right one should be");
    CHECK_MSG(!isColor(target.at(-0.5f, 0.7f), kWhite), "the left triangle reaches above y = 0.5");

    target.bind();
    glUseProgram(white.id);
    day1::drawMesh(right);
    CHECK_MSG(isColor(target.at(0.5f, -0.17f), kWhite), "nothing drawn at the center of the right triangle");
    CHECK_MSG(!isColor(target.at(-0.5f, -0.17f), kWhite), "drawing the right triangle also drew on the left");
    glUseProgram(0);
    day1::destroyMesh(left);
    day1::destroyMesh(right);
}

TEST("rectangle uses 4 vertices, 6 indices, and its VAO remembers the EBO") {
    day1::Mesh rect = day1::makeRectangle();
    REQUIRE_MSG(rect.vao && glIsVertexArray(rect.vao), "rect.vao is not a vertex array object");
    CHECK_MSG(rect.indexed && rect.count == 6, "expected indexed = true and count = 6, got " << rect.indexed << " and " << rect.count);
    CHECK_MSG(bufferSize(rect.vbo, GL_ARRAY_BUFFER) == 4 * 3 * 4,
              "VBO should hold 4 vertices = 48 bytes, has " << bufferSize(rect.vbo, GL_ARRAY_BUFFER));
    REQUIRE_MSG(rect.ebo && glIsBuffer(rect.ebo), "rect.ebo is not a buffer");

    glBindVertexArray(rect.vao);
    GLint bound = 0;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound);
    GLint indexBytes = -1;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexBytes);
    glBindVertexArray(0);
    CHECK_MSG(static_cast<GLuint>(bound) == rect.ebo,
              "binding the VAO does not bind the EBO. Did you unbind the EBO while the VAO was still bound?");
    CHECK_MSG(indexBytes == 6 * 4, "EBO should hold 6 unsigned ints = 24 bytes, has " << indexBytes);
    day1::destroyMesh(rect);
}

TEST("rectangle covers the square from -0.5 to 0.5") {
    Offscreen target;
    WhiteProgram white;
    day1::Mesh rect = day1::makeRectangle();
    REQUIRE(rect.vao);
    glUseProgram(white.id);
    day1::drawMesh(rect);
    for (glm::vec2 p : {glm::vec2(0, 0), glm::vec2(-0.4f, -0.4f), glm::vec2(0.4f, -0.4f), glm::vec2(0.4f, 0.4f),
                        glm::vec2(-0.4f, 0.4f)})
        CHECK_MSG(isColor(target.at(p.x, p.y), kWhite), "expected the rectangle at " << str(p));
    for (glm::vec2 p : {glm::vec2(-0.6f, 0), glm::vec2(0.6f, 0), glm::vec2(0, 0.6f), glm::vec2(0, -0.6f)})
        CHECK_MSG(!isColor(target.at(p.x, p.y), kWhite), "the rectangle reaches outside its square, at " << str(p));
    glUseProgram(0);
    day1::destroyMesh(rect);
}

TEST("wireframe shows two triangles sharing one diagonal") {
    Offscreen target(128, 128);
    WhiteProgram white;
    day1::Mesh rect = day1::makeRectangle();
    REQUIRE(rect.vao);
    glUseProgram(white.id);
    day1::setWireframe(true);
    day1::drawMesh(rect);

    for (glm::vec2 p : {glm::vec2(0.3f, 0.0f), glm::vec2(-0.3f, 0.0f), glm::vec2(0.0f, 0.3f), glm::vec2(0.0f, -0.3f)}) {
        const glm::ivec2 px = target.pixelOf(p.x, p.y);
        CHECK_MSG(!target.anyDrawnNear(px.x, px.y), "wireframe mode still fills the inside, at " << str(p));
    }
    for (glm::vec2 p : {glm::vec2(0.5f, 0.0f), glm::vec2(-0.5f, 0.0f), glm::vec2(0.0f, 0.5f), glm::vec2(0.0f, -0.5f)}) {
        const glm::ivec2 px = target.pixelOf(p.x, p.y);
        CHECK_MSG(target.anyDrawnNear(px.x, px.y, 2), "the outline of the square is missing near " << str(p));
    }
    int onMain = 0, onAnti = 0, samples = 0;
    for (float t = -0.4f; t <= 0.41f; t += 0.05f) {
        if (std::abs(t) < 0.15f) continue;
        ++samples;
        const glm::ivec2 a = target.pixelOf(t, t), b = target.pixelOf(t, -t);
        onMain += target.anyDrawnNear(a.x, a.y) ? 1 : 0;
        onAnti += target.anyDrawnNear(b.x, b.y) ? 1 : 0;
    }
    const bool oneDiagonal = (onMain >= samples - 1 && onAnti <= 1) || (onAnti >= samples - 1 && onMain <= 1);
    CHECK_MSG(oneDiagonal, "expected exactly one diagonal line inside the square. Lit samples on the two diagonals: "
                               << onMain << " and " << onAnti << " of " << samples);

    target.bind();
    day1::setWireframe(false);
    day1::drawMesh(rect);
    CHECK_MSG(isColor(target.at(0.3f, 0.0f), kWhite), "setWireframe(false) should fill the triangles again");
    glUseProgram(0);
    day1::destroyMesh(rect);
}

TEST("destroyMesh deletes the GL objects and resets the mesh") {
    day1::Mesh rect = day1::makeRectangle();
    REQUIRE(rect.vao);
    const day1::Mesh copy = rect;
    day1::destroyMesh(rect);
    CHECK_MSG(!glIsVertexArray(copy.vao), "the VAO still exists");
    CHECK_MSG(!glIsBuffer(copy.vbo), "the VBO still exists");
    CHECK_MSG(!glIsBuffer(copy.ebo), "the EBO still exists");
    CHECK_MSG(rect.vao == 0 && rect.vbo == 0 && rect.ebo == 0 && rect.count == 0, "the Mesh should be reset to {}");
}

// ---------------------------------------------------------------------------
// Window resize
// ---------------------------------------------------------------------------
TEST("onFramebufferResize makes the viewport cover the new framebuffer") {
    for (glm::ivec2 size : {glm::ivec2(300, 200), glm::ivec2(17, 999), glm::ivec2(2560, 1440)}) {
        day1::onFramebufferResize(size.x, size.y);
        GLint vp[4] = {-1, -1, -1, -1};
        glGetIntegerv(GL_VIEWPORT, vp);
        CHECK_MSG(vp[0] == 0 && vp[1] == 0 && vp[2] == size.x && vp[3] == size.y,
                  "after resize to " << size.x << "x" << size.y << " the viewport is " << vp[0] << "," << vp[1] << " "
                                     << vp[2] << "x" << vp[3]);
    }
}
