// Automatic checks for Day 4. Provided, don't edit.
#include "day4/camera.hpp"
#include "day4/lighting.hpp"

#include "reference_lighting.hpp"
#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <glm/gtc/matrix_transform.hpp>

SPRINT_DAY("Day 4: Fly around a lit cube");

using namespace sprint;
using sprint::test::str;

namespace {

bool finiteVec(const glm::vec3& v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }

// A 4 x 4 plane at z = 0 facing +z: position (location 0) and normal (location 1).
std::vector<float> planeWithNormals() {
    std::vector<float> v;
    const float c[6][2] = {{-2, -2}, {2, -2}, {2, 2}, {2, 2}, {-2, 2}, {-2, -2}};
    for (auto& p : c) v.insert(v.end(), {p[0], p[1], 0.0f, 0.0f, 0.0f, 1.0f});
    return v;
}

struct LitScene {
    glm::mat4 model{1.0f};
    glm::vec3 lightPos{0.5f, 0.8f, 2.0f};
    glm::vec3 viewPos{0.0f, 0.0f, 3.0f};
    glm::vec3 lightColor{1.0f, 0.9f, 0.8f};
    glm::vec3 objectColor{1.0f, 0.5f, 0.31f};
    bool blinn = false;
};

const glm::mat4 kProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
const glm::mat4 kView = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

void drawLit(const day2::Shader& shader, const TestMesh& mesh, const LitScene& s) {
    // Uniforms are set with plain GL calls, so only your shader is being tested here.
    const GLuint p = shader.id();
    glUseProgram(p);
    const glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(s.model)));
    glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_FALSE, &s.model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(p, "view"), 1, GL_FALSE, &kView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(p, "projection"), 1, GL_FALSE, &kProjection[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(p, "normalMatrix"), 1, GL_FALSE, &nm[0][0]);
    glUniform3fv(glGetUniformLocation(p, "lightPos"), 1, &s.lightPos[0]);
    glUniform3fv(glGetUniformLocation(p, "viewPos"), 1, &s.viewPos[0]);
    glUniform3fv(glGetUniformLocation(p, "lightColor"), 1, &s.lightColor[0]);
    glUniform3fv(glGetUniformLocation(p, "objectColor"), 1, &s.objectColor[0]);
    glUniform1i(glGetUniformLocation(p, "blinn"), s.blinn ? 1 : 0);
    mesh.draw();
    glUseProgram(0);
}

// Where the ray through a pixel center hits the (possibly transformed) plane.
bool hitPlane(const LitScene& s, glm::vec2 ndc, glm::vec3& pos, glm::vec3& normal) {
    const glm::vec3 p0 = glm::vec3(s.model * glm::vec4(0, 0, 0, 1));
    normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(s.model))) * glm::vec3(0, 0, 1));
    const glm::vec3 o(ndc.x, ndc.y, 3.0f), d(0, 0, -1);
    const float denom = glm::dot(d, normal);
    if (std::abs(denom) < 1e-4f) return false;
    pos = o + d * (glm::dot(p0 - o, normal) / denom);
    return true;
}

// Compares a grid of pixels with the CPU formula. Returns the number of mismatches.
int compareWithReference(const Offscreen& target, const LitScene& s, const char* what) {
    int bad = 0;
    for (int py = 6; py < target.height(); py += 13)
        for (int px = 6; px < target.width(); px += 13) {
            glm::vec3 pos, n;
            if (!hitPlane(s, target.ndcOf(px, py), pos, n)) continue;
            const glm::vec3 want = reference::phong(pos, n, s.lightPos, s.viewPos, s.lightColor, s.objectColor, s.blinn);
            const glm::vec3 got = glm::vec3(target.pixel(px, py));
            if (!test::near(got, want, 0.025f)) {
                if (bad < 3)
                    CHECK_MSG(false, what << ": pixel (" << px << ", " << py << ") is " << str(got) << ", the formula gives "
                                          << str(want) << " (world position " << str(pos) << ")");
                ++bad;
            }
        }
    if (bad > 3) CHECK_MSG(false, what << ": " << bad << " pixels differ in total");
    return bad;
}

std::unique_ptr<day2::Shader> requireLitShader() {
    auto s = day4::makeLitShader();
    REQUIRE_MSG(s && s->valid(), "lit shader did not build: " << (s ? s->error() : "null"));
    for (const char* u : {"model", "view", "projection", "normalMatrix", "lightPos", "viewPos", "lightColor", "objectColor"})
        CHECK_MSG(hasUniform(s->id(), u), "the lit shader has no active uniform \"" << u << "\" (missing, misspelled, or unused)");
    return s;
}

}  // namespace

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------
TEST("default camera looks down -z and view() matches lookAt") {
    day4::Camera cam;
    CHECK_MSG(test::near(cam.front(), {0, 0, -1}, 1e-5f), "yaw -90, pitch 0 should look along (0, 0, -1), front() is " << str(cam.front()));
    CHECK_MSG(test::near(cam.right(), {1, 0, 0}, 1e-5f), "right() should be (1, 0, 0), is " << str(cam.right()));
    const glm::mat4 want = glm::lookAt(cam.position, cam.position + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    CHECK_MSG(test::near(cam.view(), want, 1e-5f), "view() does not match glm::lookAt(position, position + front, up)");
}

TEST("front() follows yaw and pitch and stays a unit vector") {
    day4::Camera cam;
    cam.yaw = 0.0f;
    CHECK_MSG(test::near(cam.front(), {1, 0, 0}, 1e-5f), "yaw 0 should look along +x, front() is " << str(cam.front()));
    cam.yaw = -90.0f;
    cam.pitch = 30.0f;
    CHECK_MSG(test::near(cam.front(), {0, 0.5f, -0.8660254f}, 1e-4f), "pitch 30 should look up: y = sin(30) = 0.5, got " << str(cam.front()));
    cam.yaw = 37.0f;
    cam.pitch = -61.0f;
    CHECK_MSG(test::near(glm::length(cam.front()), 1.0f, 1e-4f), "front() should have length 1");
    CHECK_MSG(test::near(glm::dot(cam.front(), cam.right()), 0.0f, 1e-4f) && test::near(cam.right().y, 0.0f, 1e-4f),
              "right() should be perpendicular to front() and horizontal");
}

TEST("move() goes speed x time along the right direction") {
    day4::Camera cam;
    cam.move(day4::Move::Forward, 1.0f);
    CHECK_MSG(test::near(cam.position, {0, 0, 0.5f}, 1e-4f), "Forward for 1 s at speed 2.5 from (0,0,3) should reach (0,0,0.5), got " << str(cam.position));
    cam = day4::Camera{};
    cam.move(day4::Move::Right, 0.4f);
    CHECK_MSG(test::near(cam.position, {1, 0, 3}, 1e-4f), "Right for 0.4 s should reach (1,0,3), got " << str(cam.position));
    cam = day4::Camera{};
    cam.move(day4::Move::Backward, 0.4f);
    cam.move(day4::Move::Left, 0.4f);
    cam.move(day4::Move::Up, 0.4f);
    CHECK_MSG(test::near(cam.position, {-1, 1, 4}, 1e-4f), "Backward, Left, Up 0.4 s each should reach (-1,1,4), got " << str(cam.position));
    cam.move(day4::Move::Down, 0.4f);
    CHECK_MSG(test::near(cam.position.y, 0.0f, 1e-4f), "Down should move along -y");
    cam = day4::Camera{};
    cam.pitch = 45.0f;
    cam.move(day4::Move::Up, 1.0f);
    CHECK_MSG(test::near(cam.position, {0, 2.5f, 3}, 1e-4f), "Up should use the world up, not the camera's tilt");
}

TEST("movement is the same at 30 fps and 300 fps") {
    day4::Camera a, b;
    for (int i = 0; i < 30; ++i) a.move(day4::Move::Forward, 1.0f / 30.0f);
    for (int i = 0; i < 300; ++i) b.move(day4::Move::Forward, 1.0f / 300.0f);
    CHECK_MSG(test::near(a.position, b.position, 1e-3f) && test::near(a.position.z, 0.5f, 1e-3f),
              "after 1 s, 30 fps reached " << str(a.position) << " and 300 fps reached " << str(b.position));
}

TEST("look() turns with the mouse and clamps pitch") {
    day4::Camera cam;
    cam.look(100.0f, 0.0f);
    CHECK_MSG(test::near(cam.yaw, -80.0f, 1e-3f), "moving the mouse 100 px right should add 10 degrees of yaw, yaw is " << cam.yaw);
    cam.look(0.0f, -100.0f);
    CHECK_MSG(test::near(cam.pitch, 10.0f, 1e-3f), "moving the mouse 100 px up (dy = -100) should look up 10 degrees, pitch is " << cam.pitch);
    cam.look(0.0f, -100000.0f);
    CHECK_MSG(test::near(cam.pitch, 89.0f, 1e-3f), "pitch should stop at 89, is " << cam.pitch);
    CHECK_MSG(finiteVec(cam.front()) && test::finite(cam.view()), "looking straight up gives NaN in front() or view()");
    cam.look(0.0f, 100000.0f);
    CHECK_MSG(test::near(cam.pitch, -89.0f, 1e-3f), "pitch should stop at -89, is " << cam.pitch);
}

TEST("zoom() changes the field of view between 1 and 45") {
    day4::Camera cam;
    cam.zoom(5.0f);
    CHECK_MSG(test::near(cam.fov, 40.0f), "scrolling up 5 should zoom in to fov 40, fov is " << cam.fov);
    cam.zoom(-100.0f);
    CHECK_MSG(test::near(cam.fov, 45.0f), "fov should not go above 45, is " << cam.fov);
    cam.zoom(100.0f);
    CHECK_MSG(test::near(cam.fov, 1.0f), "fov should not go below 1, is " << cam.fov);
}

TEST("MouseTracker ignores the first position, then reports movement") {
    day4::MouseTracker m;
    CHECK_MSG(test::near(m.delta(500.0, 300.0), glm::vec2(0, 0)), "the first call should return (0, 0)");
    CHECK_MSG(test::near(m.delta(510.0, 290.0), glm::vec2(10, -10)), "moving to (510, 290) should report (10, -10)");
    CHECK_MSG(test::near(m.delta(510.0, 290.0), glm::vec2(0, 0)), "not moving should report (0, 0)");
}

// ---------------------------------------------------------------------------
// Normals
// ---------------------------------------------------------------------------
TEST("normalMatrix keeps normals perpendicular under non-uniform scale") {
    const glm::mat4 model = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(1, 3, 0.5f)), glm::radians(30.0f),
                                        glm::normalize(glm::vec3(1, 1, 0)));
    const glm::mat3 nm = day4::normalMatrix(model);
    const glm::vec3 n(0, 0, 1), t1(1, 0, 0), t2(0, 1, 0);  // a surface normal and two directions along the surface
    const glm::vec3 nw = nm * n, t1w = glm::mat3(model) * t1, t2w = glm::mat3(model) * t2;
    CHECK_MSG(test::near(glm::dot(glm::normalize(nw), glm::normalize(t1w)), 0.0f, 1e-4f) &&
                  test::near(glm::dot(glm::normalize(nw), glm::normalize(t2w)), 0.0f, 1e-4f),
              "the transformed normal is no longer perpendicular to the surface. Use transpose(inverse(mat3(model)))");
    CHECK_MSG(test::near(day4::normalMatrix(glm::translate(glm::mat4(1), glm::vec3(5, 6, 7))), glm::mat3(1.0f)),
              "translation must not change normals");
}

// ---------------------------------------------------------------------------
// Shaders
// ---------------------------------------------------------------------------
TEST("light cube shader draws white") {
    Offscreen target;
    auto s = day4::makeLightCubeShader();
    REQUIRE_MSG(s && s->valid(), "light shader did not build: " << (s ? s->error() : "null"));
    TestMesh screen(fullscreenTriangle(), {3});
    const glm::mat4 id(1.0f);
    glUseProgram(s->id());
    for (const char* u : {"model", "view", "projection"})
        glUniformMatrix4fv(glGetUniformLocation(s->id(), u), 1, GL_FALSE, &id[0][0]);
    screen.draw();
    glUseProgram(0);
    CHECK_MSG(test::near(target.at(0, 0), glm::vec4(1), kPixelEps), "expected white, got " << str(target.at(0, 0)));
}

TEST("Phong lighting matches the formula (light in front)") {
    Offscreen target;
    auto shader = requireLitShader();
    TestMesh plane(planeWithNormals(), {3, 3});
    LitScene s;
    glEnable(GL_DEPTH_TEST);
    drawLit(*shader, plane, s);
    compareWithReference(target, s, "light in front");
}

TEST("only ambient light reaches a surface lit from behind") {
    Offscreen target;
    auto shader = requireLitShader();
    TestMesh plane(planeWithNormals(), {3, 3});
    LitScene s;
    s.lightPos = {0.3f, -0.2f, -2.0f};
    drawLit(*shader, plane, s);
    const glm::vec3 ambientOnly = 0.1f * s.lightColor * s.objectColor;
    CHECK_MSG(test::near(glm::vec3(target.at(0.4f, 0.4f)), ambientOnly, 0.02f),
              "expected only ambient " << str(ambientOnly) << ", got " << str(glm::vec3(target.at(0.4f, 0.4f)))
                                       << ". Clamp the diffuse and specular dot products with max(..., 0.0)");
}

TEST("the highlight moves with the camera, the diffuse part doesn't") {
    Offscreen target;
    auto shader = requireLitShader();
    TestMesh plane(planeWithNormals(), {3, 3});
    LitScene s;
    s.lightPos = {0.0f, 0.0f, 1.5f};
    s.viewPos = {1.2f, 0.4f, 2.0f};
    drawLit(*shader, plane, s);
    compareWithReference(target, s, "camera moved to (1.2, 0.4, 2)");
}

TEST("Blinn-Phong matches the formula") {
    Offscreen target;
    auto shader = requireLitShader();
    TestMesh plane(planeWithNormals(), {3, 3});
    LitScene s;
    s.blinn = true;
    drawLit(*shader, plane, s);
    compareWithReference(target, s, "Blinn-Phong");
    REQUIRE(hasUniform(shader->id(), "blinn"));
}

TEST("a tilted, non-uniformly scaled surface is lit correctly") {
    Offscreen target;
    auto shader = requireLitShader();
    TestMesh plane(planeWithNormals(), {3, 3});
    LitScene s;
    s.model = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(1, 3, 1)), glm::radians(45.0f), glm::vec3(1, 0, 0));
    s.lightPos = {0.2f, -1.5f, 2.0f};
    glEnable(GL_DEPTH_TEST);
    drawLit(*shader, plane, s);
    compareWithReference(target, s, "scaled (1, 3, 1) and tilted 45 degrees");
}
