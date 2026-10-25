// Automatic checks for Day 3. Provided, don't edit.
#include "day2/shader.hpp"
#include "day3/cube.hpp"
#include "day3/transforms.hpp"

#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <glm/gtc/matrix_transform.hpp>

SPRINT_DAY("Day 3: Ten spinning crates");

using namespace sprint;
using sprint::test::str;

namespace {

glm::vec3 apply(const glm::mat4& m, const glm::vec3& p) { return glm::vec3(m * glm::vec4(p, 1.0f)); }
glm::vec3 project(const glm::mat4& m, const glm::vec3& p) {
    const glm::vec4 c = m * glm::vec4(p, 1.0f);
    return glm::vec3(c) / c.w;
}
bool isColor(const glm::vec4& got, const glm::vec4& want, float eps = kPixelEps) { return test::near(got, want, eps); }

const char* kDepthVS = R"(#version 330 core
layout(location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); })";
const char* kDepthFS = R"(#version 330 core
uniform vec4 color;
out vec4 c;
void main() { c = color; })";

const glm::vec4 kRed{1, 0, 0, 1}, kGreen{0, 1, 0, 1}, kBlue{0, 0, 1, 1}, kWhite{1};
const glm::vec4 kFrameClear{0.1f, 0.12f, 0.16f, 1.0f};

struct ColorProgram {
    GLuint id = buildTestProgram(kDepthVS, kDepthFS);
    ~ColorProgram() { glDeleteProgram(id); }
    void use(const glm::vec4& c) const {
        glUseProgram(id);
        glUniform4fv(glGetUniformLocation(id, "color"), 1, &c[0]);
    }
};

const char* kMatVS = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uM;
uniform mat3 uN;
void main() { gl_Position = uM * vec4(uN * aPos, 1.0); })";
const char* kMatFS = R"(#version 330 core
out vec4 c;
void main() { c = vec4(1.0); })";

}  // namespace

// ---------------------------------------------------------------------------
// Matrices
// ---------------------------------------------------------------------------
TEST("makeModel scales first, then rotates, then translates") {
    const glm::mat4 m = day3::makeModel({1, 0, 0}, glm::radians(90.0f), {0, 0, 1}, {2, 1, 1});
    const glm::vec3 got = apply(m, {1, 0, 0});
    // scale (1,0,0) -> (2,0,0), rotate 90 degrees around z -> (0,2,0), translate -> (1,2,0)
    CHECK_MSG(test::near(got, {1, 2, 0}, 1e-4f),
              "the point (1,0,0) should end up at (1, 2, 0), it ends up at "
                  << str(got) << ". Check the order: the call written last in code is applied first");
    const glm::mat4 n = day3::makeModel({0, 0, 0}, 0.0f, {0, 1, 0}, {1, 1, 1});
    CHECK_MSG(test::near(n, glm::mat4(1.0f)), "no move, no rotation and scale 1 should give the identity matrix, got " << str(n));
    const glm::mat4 r = day3::makeModel({3, -1, 2}, 0.7f, glm::normalize(glm::vec3(1, 0.3f, 0.5f)), {0.5f, 2, 1});
    const glm::mat4 want = glm::scale(glm::rotate(glm::translate(glm::mat4(1), glm::vec3(3, -1, 2)), 0.7f,
                                                  glm::normalize(glm::vec3(1, 0.3f, 0.5f))),
                                      glm::vec3(0.5f, 2, 1));
    CHECK_MSG(test::near(r, want, 1e-4f), "makeModel does not match translate * rotate * scale");
}

TEST("perspective projection uses the framebuffer's aspect ratio") {
    const glm::mat4 p = day3::makeProjection(true, 45.0f, 1600, 900, 0.1f, 100.0f);
    const glm::mat4 want = glm::perspective(glm::radians(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
    CHECK_MSG(test::near(p, want, 1e-5f), "expected glm::perspective(glm::radians(45), 1600/900, 0.1, 100), got " << str(p));
    const glm::mat4 tall = day3::makeProjection(true, 60.0f, 500, 1000, 0.5f, 50.0f);
    CHECK_MSG(test::near(tall, glm::perspective(glm::radians(60.0f), 0.5f, 0.5f, 50.0f), 1e-5f),
              "a 500x1000 framebuffer should use aspect 0.5. Watch out for integer division");
}

TEST("projection survives a minimized window (height 0)") {
    REQUIRE_MSG(test::near(day3::makeProjection(true, 45.0f, 800, 600, 0.1f, 100.0f),
                           glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f), 1e-5f),
                "the normal perspective projection must work first");
    const glm::mat4 p = day3::makeProjection(true, 45.0f, 800, 0, 0.1f, 100.0f);
    const glm::mat4 o = day3::makeProjection(false, 45.0f, 800, 0, 0.1f, 100.0f);
    CHECK_MSG(test::finite(p), "perspective matrix has inf or NaN for height 0: " << str(p));
    CHECK_MSG(test::finite(o), "orthographic matrix has inf or NaN for height 0: " << str(o));
}

TEST("orthographic projection keeps sizes the same at every distance") {
    const glm::mat4 o = day3::makeProjection(false, 45.0f, 800, 400, 0.1f, 100.0f, 3.0f);
    const glm::vec3 nearPoint = project(o, {1, 1, -2}), farPoint = project(o, {1, 1, -40});
    CHECK_MSG(test::near(glm::vec2(nearPoint), glm::vec2(farPoint), 1e-5f),
              "a point at distance 2 and at distance 40 should land on the same x, y. Got " << str(nearPoint) << " and "
                                                                                            << str(farPoint));
    CHECK_MSG(test::near(project(o, {0, 3, -5}).y, 1.0f, 1e-4f), "y = orthoHalfHeight (3) should map to the top edge");
    CHECK_MSG(test::near(project(o, {6, 0, -5}).x, 1.0f, 1e-4f),
              "with aspect 2, x = 6 should map to the right edge. Got " << project(o, {6, 0, -5}).x);
    const glm::vec3 pa = project(day3::makeProjection(true, 45.0f, 800, 400, 0.1f, 100.0f), {1, 1, -2});
    const glm::vec3 pb = project(day3::makeProjection(true, 45.0f, 800, 400, 0.1f, 100.0f), {1, 1, -40});
    CHECK_MSG(std::abs(pa.x) > std::abs(pb.x) * 5.0f, "with perspective, the far point should be much closer to the center");
}

TEST("Shader::setMat4 and setMat3 upload matrices unchanged") {
    const std::string dir = tempDir();
    writeFile(dir + "/m.vert", kMatVS);
    writeFile(dir + "/m.frag", kMatFS);
    day2::Shader s(dir + "/m.vert", dir + "/m.frag");
    REQUIRE_MSG(s.valid(), s.error());
    glm::mat4 m(0.0f);
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r) m[c][r] = static_cast<float>(c * 4 + r + 1);
    glm::mat3 n(0.0f);
    for (int c = 0; c < 3; ++c)
        for (int r = 0; r < 3; ++r) n[c][r] = static_cast<float>(10 + c * 3 + r);
    s.use();
    s.setMat4("uM", m);
    s.setMat3("uN", n);
    const glm::mat4 gotM = uniformMat4(s.id(), "uM");
    CHECK_MSG(test::near(gotM, m), "setMat4 uploaded " << str(gotM) << " instead of " << str(m)
                                                       << ". Use transpose = GL_FALSE and glm::value_ptr");
    CHECK_MSG(test::near(uniformMat3(s.id(), "uN"), n), "setMat3 uploaded the wrong values: " << str(uniformMat3(s.id(), "uN")));
    glUseProgram(0);
}

TEST("cube angles grow with time, at 10 different speeds") {
    float speeds[10];
    for (int i = 0; i < 10; ++i) {
        CHECK_MSG(test::near(day3::cubeAngle(i, 0.0), 0.0f), "cube " << i << " should have angle 0 at time 0");
        const float a1 = day3::cubeAngle(i, 1.0), a2 = day3::cubeAngle(i, 2.0), a5 = day3::cubeAngle(i, 5.0);
        CHECK_MSG(test::near(a2, 2.0f * a1, 1e-4f) && test::near(a5, 5.0f * a1, 1e-3f),
                  "cube " << i << ": the angle must be proportional to time (per second, not per frame)");
        speeds[i] = a1;
        CHECK_MSG(std::abs(a1) > 1e-3f, "cube " << i << " does not spin");
    }
    for (int i = 0; i < 10; ++i)
        for (int j = i + 1; j < 10; ++j)
            CHECK_MSG(!test::near(speeds[i], speeds[j], 1e-4f), "cubes " << i << " and " << j << " spin at the same speed");
}

// ---------------------------------------------------------------------------
// Depth buffer
// ---------------------------------------------------------------------------
TEST("beginFrame clears to the frame color") {
    Offscreen target;
    day3::beginFrame(true);
    CHECK_MSG(isColor(target.at(0, 0), kFrameClear), "after beginFrame the screen should be " << str(kFrameClear) << ", got "
                                                                                             << str(target.at(0, 0)));
}

TEST("with the depth test on, the nearer surface wins") {
    Offscreen target;
    ColorProgram prog;
    TestMesh nearQuad(fullscreenTriangle(-0.5f), {3}), farQuad(fullscreenTriangle(0.5f), {3});
    day3::beginFrame(true);
    prog.use(kRed);
    nearQuad.draw();
    prog.use(kGreen);
    farQuad.draw();
    CHECK_MSG(isColor(target.at(0, 0), kRed), "the far green surface was drawn over the near red one. Enable GL_DEPTH_TEST");
    glUseProgram(0);
}

TEST("with the depth test off, the last drawn surface wins") {
    Offscreen target;
    ColorProgram prog;
    TestMesh nearQuad(fullscreenTriangle(-0.5f), {3}), farQuad(fullscreenTriangle(0.5f), {3});
    glEnable(GL_DEPTH_TEST);
    day3::beginFrame(false);
    prog.use(kRed);
    nearQuad.draw();
    prog.use(kGreen);
    farQuad.draw();
    CHECK_MSG(isColor(target.at(0, 0), kGreen), "beginFrame(false) should disable the depth test");
    glUseProgram(0);
}

TEST("beginFrame clears the depth buffer every frame") {
    Offscreen target;
    ColorProgram prog;
    TestMesh nearQuad(fullscreenTriangle(-0.5f), {3}), farQuad(fullscreenTriangle(0.5f), {3});
    day3::beginFrame(true);
    REQUIRE_MSG(glIsEnabled(GL_DEPTH_TEST), "beginFrame(true) should enable the depth test");
    prog.use(kRed);
    nearQuad.draw();
    day3::beginFrame(true);  // next frame: only the far surface
    prog.use(kGreen);
    farQuad.draw();
    CHECK_MSG(isColor(target.at(0, 0), kGreen),
              "the second frame's surface was hidden by the first frame's depth. Clear GL_DEPTH_BUFFER_BIT too");
    glUseProgram(0);
}

// ---------------------------------------------------------------------------
// Cube
// ---------------------------------------------------------------------------
TEST("cube has 36 vertices of 5 floats with attributes 0 and 1") {
    day3::Cube cube = day3::makeCube();
    REQUIRE_MSG(cube.vao && glIsVertexArray(cube.vao), "makeCube did not create a VAO");
    CHECK_MSG(cube.count == 36, "count should be 36, is " << cube.count);
    GLint bytes = 0;
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bytes);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_MSG(bytes == 36 * 5 * 4, "VBO should hold 36 * 5 floats = 720 bytes, has " << bytes);
    glBindVertexArray(cube.vao);
    for (int loc = 0; loc < 2; ++loc) {
        GLint enabled = 0, size = 0, stride = 0;
        void* offset = nullptr;
        glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
        glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
        glGetVertexAttribPointerv(loc, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
        CHECK_MSG(enabled == GL_TRUE, "attribute " << loc << " is not enabled");
        CHECK_MSG(size == (loc == 0 ? 3 : 2), "attribute " << loc << " should have " << (loc == 0 ? 3 : 2) << " components");
        CHECK_MSG(stride == 20, "attribute " << loc << " stride should be 20 bytes, is " << stride);
        CHECK_MSG(reinterpret_cast<std::uintptr_t>(offset) == (loc == 0 ? 0u : 12u),
                  "attribute " << loc << " offset should be " << (loc == 0 ? 0 : 12) << " bytes");
    }
    glBindVertexArray(0);
    day3::destroyCube(cube);
}

TEST("cube is drawn from -0.5 to 0.5 with the texture upright on the front face") {
    Offscreen target;
    auto shader = day3::makeCubeShader();
    REQUIRE_MSG(shader && shader->valid(), "cube shader did not build: " << (shader ? shader->error() : "null"));
    day3::Cube cube = day3::makeCube();
    REQUIRE(cube.vao);
    // bottom row first: bottom-left red, bottom-right green, top-left blue, top-right white
    GLuint tex = makeTexture(2, 2, {kRed, kGreen, kBlue, kWhite}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    const glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
    const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
    glEnable(GL_DEPTH_TEST);
    day3::drawCube(cube, *shader, glm::mat4(1.0f), view, projection, tex);
    CHECK_MSG(isColor(target.at(-0.25f, -0.25f), kRed), "front face bottom-left should be red, got " << str(target.at(-0.25f, -0.25f)));
    CHECK_MSG(isColor(target.at(0.25f, -0.25f), kGreen), "front face bottom-right should be green, got " << str(target.at(0.25f, -0.25f)));
    CHECK_MSG(isColor(target.at(-0.25f, 0.25f), kBlue), "front face top-left should be blue, got " << str(target.at(-0.25f, 0.25f)));
    CHECK_MSG(isColor(target.at(0.25f, 0.25f), kWhite), "front face top-right should be white, got " << str(target.at(0.25f, 0.25f)));
    CHECK_MSG(isColor(target.at(0.75f, 0.0f), kTestClearColor) && isColor(target.at(0.0f, -0.75f), kTestClearColor),
              "the cube is bigger than -0.5..0.5, or the matrices are not applied");

    // Moved right by 0.5 with the model matrix
    target.bind();
    day3::drawCube(cube, *shader, glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0, 0)), view, projection, tex);
    CHECK_MSG(isColor(target.at(0.9f, -0.25f), kGreen) && isColor(target.at(-0.25f, 0.0f), kTestClearColor),
              "a model matrix that moves the cube right by 0.5 had no effect. Is the shader using model, view and projection?");
    glDeleteTextures(1, &tex);
    day3::destroyCube(cube);
}

TEST("perspective cube: the camera's distance changes its size") {
    Offscreen target;
    auto shader = day3::makeCubeShader();
    REQUIRE(shader && shader->valid());
    day3::Cube cube = day3::makeCube();
    REQUIRE(cube.vao);
    GLuint tex = makeSolidTexture(kWhite);
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    auto widthAt = [&](float distance) {
        target.bind();
        glEnable(GL_DEPTH_TEST);
        day3::drawCube(cube, *shader, glm::mat4(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -distance)),
                       projection, tex);
        int w = 0;
        for (int x = 0; x < target.width(); ++x)
            if (isColor(target.pixel(x, target.height() / 2), kWhite)) ++w;
        return w;
    };
    const int nearW = widthAt(2.0f), farW = widthAt(6.0f);
    CHECK_MSG(nearW > farW * 2 && farW > 0, "at distance 2 the cube should look much wider than at distance 6. Widths in pixels: "
                                                << nearW << " and " << farW);
    glDeleteTextures(1, &tex);
    day3::destroyCube(cube);
}
