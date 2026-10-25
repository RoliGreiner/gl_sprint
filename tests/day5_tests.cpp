// Automatic checks for Day 5. Provided, don't edit.
#include "day5/lights.hpp"

#include "reference_lighting.hpp"
#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <glm/gtc/matrix_transform.hpp>

SPRINT_DAY("Day 5: The lit crate yard");

using namespace sprint;
using sprint::test::str;

namespace {

const glm::vec3 kDiffuseColor{0.8f, 0.6f, 0.4f};
const glm::mat4 kProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
const glm::vec3 kViewPos{0.0f, 0.0f, 3.0f};
const glm::mat4 kView = glm::lookAt(kViewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

// 4 x 4 plane at z = 0 facing +z: position, normal, texture coordinate (0..1 across the plane).
std::vector<float> plane() {
    std::vector<float> v;
    const float c[6][2] = {{-2, -2}, {2, -2}, {2, 2}, {2, 2}, {-2, 2}, {-2, -2}};
    for (auto& p : c) v.insert(v.end(), {p[0], p[1], 0.0f, 0.0f, 0.0f, 1.0f, (p[0] + 2) / 4, (p[1] + 2) / 4});
    return v;
}

struct Fixture {
    Offscreen target;
    std::unique_ptr<day2::Shader> shader;
    TestMesh mesh{plane(), {3, 3, 2}};
    day5::Material material;

    Fixture() {
        shader = day5::makeSceneShader();
        REQUIRE_MSG(shader && shader->valid(), "scene shader did not build: " << (shader ? shader->error() : "null"));
        material.diffuseMap = makeSolidTexture(glm::vec4(kDiffuseColor, 1.0f));
        // left half of the plane is shiny (white), right half is not (black)
        material.specularMap = makeTexture(2, 1, {glm::vec4(1), glm::vec4(0, 0, 0, 1)}, GL_NEAREST, GL_CLAMP_TO_EDGE);
        material.shininess = 32.0f;
    }
    ~Fixture() {
        glDeleteTextures(1, &material.diffuseMap);
        glDeleteTextures(1, &material.specularMap);
    }

    // Uploads the camera with plain GL calls and the lights with your functions, then draws.
    void draw(const day5::DirLight& dir, const std::vector<day5::PointLight>& points, int count,
              const day5::SpotLight& spot, bool spotOn) {
        target.bind();
        shader->use();
        const GLuint p = shader->id();
        const glm::mat4 model(1.0f);
        const glm::mat3 nm(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(p, "view"), 1, GL_FALSE, &kView[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(p, "projection"), 1, GL_FALSE, &kProjection[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(p, "normalMatrix"), 1, GL_FALSE, &nm[0][0]);
        glUniform3fv(glGetUniformLocation(p, "viewPos"), 1, &kViewPos[0]);
        day5::applyMaterial(*shader, material);
        day5::uploadDirLight(*shader, dir);
        for (int i = 0; i < day5::kMaxPointLights; ++i)
            day5::uploadPointLight(*shader, i < static_cast<int>(points.size()) ? points[i] : off<day5::PointLight>(), i);
        day5::uploadPointLightCount(*shader, count);
        day5::uploadSpotLight(*shader, spot, spotOn);
        mesh.draw();
        glUseProgram(0);
    }

    template <class T>
    static T off() {
        T l;
        l.ambient = l.diffuse = l.specular = glm::vec3(0.0f);
        return l;
    }
};

reference::Dir ref(const day5::DirLight& l) { return {l.direction, l.ambient, l.diffuse, l.specular}; }
reference::Point ref(const day5::PointLight& l) {
    return {l.position, l.constant, l.linear, l.quadratic, l.ambient, l.diffuse, l.specular};
}
reference::Spot ref(const day5::SpotLight& l) {
    return {l.position,  l.direction, std::cos(glm::radians(l.innerCutoffDegrees)), std::cos(glm::radians(l.outerCutoffDegrees)),
            l.constant,  l.linear,    l.quadratic, l.ambient, l.diffuse, l.specular};
}

glm::vec3 expected(glm::vec2 ndc, const day5::DirLight& dir, const std::vector<day5::PointLight>& points, int count,
                   const day5::SpotLight& spot, bool spotOn) {
    reference::Surface s{glm::vec3(ndc, 0.0f), {0, 0, 1}, kDiffuseColor, glm::vec3(ndc.x < 0.0f ? 1.0f : 0.0f), 32.0f};
    const glm::vec3 V = glm::normalize(kViewPos - s.pos);
    glm::vec3 c = reference::dirLight(ref(dir), s, V);
    for (int i = 0; i < count && i < static_cast<int>(points.size()); ++i) c += reference::pointLight(ref(points[i]), s, V);
    if (spotOn) c += reference::spotLight(ref(spot), s, V);
    return reference::saturate(c);
}

void compare(Fixture& f, const day5::DirLight& dir, const std::vector<day5::PointLight>& points, int count,
             const day5::SpotLight& spot, bool spotOn, const char* what) {
    f.draw(dir, points, count, spot, spotOn);
    int bad = 0;
    for (int py = 4; py < f.target.height(); py += 9)
        for (int px = 4; px < f.target.width(); px += 9) {
            const glm::vec2 ndc = f.target.ndcOf(px, py);
            if (std::abs(ndc.x) < 0.06f) continue;  // the shiny / dull border of the specular map
            const glm::vec3 want = expected(ndc, dir, points, count, spot, spotOn);
            const glm::vec3 got(f.target.pixel(px, py));
            if (!test::near(got, want, 0.025f)) {
                if (bad < 3)
                    CHECK_MSG(false, what << ": pixel at " << str(ndc) << " is " << str(got) << ", the formula gives " << str(want));
                ++bad;
            }
        }
    if (bad > 3) CHECK_MSG(false, what << ": " << bad << " pixels differ in total");
}

}  // namespace

// ---------------------------------------------------------------------------
// Uploading uniforms
// ---------------------------------------------------------------------------
TEST("applyMaterial sets the samplers and binds the two maps") {
    Fixture f;
    f.shader->use();
    f.material.shininess = 64.0f;
    day5::applyMaterial(*f.shader, f.material);
    const GLuint p = f.shader->id();
    CHECK_MSG(uniformInt(p, "material.diffuse") == 0, "material.diffuse should be texture unit 0");
    CHECK_MSG(uniformInt(p, "material.specular") == 1, "material.specular should be texture unit 1");
    CHECK_MSG(test::near(uniformFloat(p, "material.shininess"), 64.0f), "material.shininess should be 64");
    GLint bound0 = 0, bound1 = 0;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound0);
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound1);
    glActiveTexture(GL_TEXTURE0);
    CHECK_MSG(static_cast<GLuint>(bound0) == f.material.diffuseMap, "the diffuse map is not bound on texture unit 0");
    CHECK_MSG(static_cast<GLuint>(bound1) == f.material.specularMap, "the specular map is not bound on texture unit 1");
    glUseProgram(0);
}

TEST("uploadDirLight sets dirLight") {
    Fixture f;
    f.shader->use();
    day5::DirLight l;
    l.direction = {0.1f, -0.9f, 0.2f};
    l.ambient = {0.01f, 0.02f, 0.03f};
    l.diffuse = {0.4f, 0.5f, 0.6f};
    l.specular = {0.7f, 0.8f, 0.9f};
    day5::uploadDirLight(*f.shader, l);
    const GLuint p = f.shader->id();
    CHECK_MSG(test::near(uniformVec3(p, "dirLight.direction"), l.direction), "dirLight.direction is " << str(uniformVec3(p, "dirLight.direction")));
    CHECK(test::near(uniformVec3(p, "dirLight.ambient"), l.ambient));
    CHECK(test::near(uniformVec3(p, "dirLight.diffuse"), l.diffuse));
    CHECK(test::near(uniformVec3(p, "dirLight.specular"), l.specular));
    glUseProgram(0);
}

TEST("uploadPointLight sets pointLights[index] and the count") {
    Fixture f;
    f.shader->use();
    day5::PointLight l;
    l.position = {1, 2, 3};
    l.constant = 1.5f;
    l.linear = 0.2f;
    l.quadratic = 0.07f;
    l.ambient = {0.1f, 0.0f, 0.0f};
    l.diffuse = {0.0f, 0.6f, 0.0f};
    l.specular = {0.0f, 0.0f, 0.9f};
    day5::uploadPointLight(*f.shader, l, 2);
    day5::uploadPointLightCount(*f.shader, 3);
    const GLuint p = f.shader->id();
    CHECK_MSG(test::near(uniformVec3(p, "pointLights[2].position"), l.position), "pointLights[2].position is " << str(uniformVec3(p, "pointLights[2].position")));
    CHECK(test::near(uniformFloat(p, "pointLights[2].constant"), 1.5f));
    CHECK(test::near(uniformFloat(p, "pointLights[2].linear"), 0.2f));
    CHECK(test::near(uniformFloat(p, "pointLights[2].quadratic"), 0.07f));
    CHECK(test::near(uniformVec3(p, "pointLights[2].ambient"), l.ambient));
    CHECK(test::near(uniformVec3(p, "pointLights[2].diffuse"), l.diffuse));
    CHECK(test::near(uniformVec3(p, "pointLights[2].specular"), l.specular));
    CHECK_MSG(!test::near(uniformVec3(p, "pointLights[1].position"), l.position), "index 2 was written to pointLights[1]");
    CHECK_MSG(uniformInt(p, "numPointLights") == 3, "numPointLights should be 3");
    glUseProgram(0);
}

TEST("uploadSpotLight sends the cutoff angles as cosines") {
    Fixture f;
    f.shader->use();
    day5::SpotLight l;
    l.position = {0.5f, 1.0f, 2.0f};
    l.direction = {0.0f, -0.5f, -1.0f};
    l.innerCutoffDegrees = 12.5f;
    l.outerCutoffDegrees = 17.5f;
    day5::uploadSpotLight(*f.shader, l, true);
    const GLuint p = f.shader->id();
    CHECK(test::near(uniformVec3(p, "spotLight.position"), l.position));
    CHECK(test::near(uniformVec3(p, "spotLight.direction"), l.direction));
    const float c = uniformFloat(p, "spotLight.cutOff"), o = uniformFloat(p, "spotLight.outerCutOff");
    CHECK_MSG(test::near(c, std::cos(glm::radians(12.5f)), 1e-4f),
              "spotLight.cutOff should be cos(12.5 degrees) = " << std::cos(glm::radians(12.5f)) << ", is " << c);
    CHECK_MSG(test::near(o, std::cos(glm::radians(17.5f)), 1e-4f),
              "spotLight.outerCutOff should be cos(17.5 degrees) = " << std::cos(glm::radians(17.5f)) << ", is " << o);
    CHECK(test::near(uniformFloat(p, "spotLight.constant"), l.constant));
    CHECK(test::near(uniformFloat(p, "spotLight.linear"), l.linear));
    CHECK(test::near(uniformFloat(p, "spotLight.quadratic"), l.quadratic));
    CHECK(test::near(uniformVec3(p, "spotLight.diffuse"), l.diffuse));
    CHECK_MSG(uniformInt(p, "spotLightOn") == 1, "spotLightOn should be 1 (true)");
    day5::uploadSpotLight(*f.shader, l, false);
    CHECK_MSG(uniformInt(p, "spotLightOn") == 0, "spotLightOn should be 0 (false) after uploadSpotLight(..., false)");
    glUseProgram(0);
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
TEST("directional light and the specular map match the formula") {
    Fixture f;
    day5::DirLight sun;
    sun.direction = {0.1f, -0.2f, -1.0f};
    sun.ambient = glm::vec3(0.1f);
    sun.diffuse = glm::vec3(0.5f);
    sun.specular = glm::vec3(1.0f);
    compare(f, sun, {}, 0, Fixture::off<day5::SpotLight>(), false, "directional light");
    // The highlight must only appear on the shiny half.
    const glm::vec3 shiny(f.target.at(-0.15f, 0.1f)), dull(f.target.at(0.15f, 0.1f));
    CHECK_MSG(glm::length(shiny - dull) > 0.05f, "the shiny left half and the dull right half look the same ("
                                                     << str(shiny) << " vs " << str(dull) << "). Is the specular map used?");
}

TEST("point lights fade with distance and only numPointLights are used") {
    Fixture f;
    day5::PointLight bulb;
    bulb.position = {0.5f, 0.2f, 1.0f};
    bulb.ambient = glm::vec3(0.05f);
    bulb.diffuse = {0.9f, 0.8f, 0.7f};
    bulb.specular = glm::vec3(1.0f);
    day5::PointLight ignored = bulb;
    ignored.position = {-0.6f, -0.5f, 0.5f};
    ignored.diffuse = glm::vec3(5.0f);
    const auto dark = Fixture::off<day5::DirLight>();
    const auto noSpot = Fixture::off<day5::SpotLight>();
    compare(f, dark, {bulb, ignored}, 1, noSpot, false, "one point light (a second one exists but count is 1)");

    day5::PointLight farBulb = bulb;
    farBulb.position = {0.5f, 0.2f, 1.0f};
    farBulb.linear = 0.7f;
    farBulb.quadratic = 1.8f;
    compare(f, dark, {farBulb}, 1, noSpot, false, "a short-range point light");

    f.draw(dark, {bulb}, 0, noSpot, false);
    CHECK_MSG(test::near(glm::vec3(f.target.at(0.5f, 0.2f)), glm::vec3(0.0f), kPixelEps),
              "with numPointLights = 0 and no other light the plane should be black, got " << str(f.target.at(0.5f, 0.2f)));
}

TEST("the spotlight has a soft edge and turns off") {
    Fixture f;
    day5::SpotLight torch;
    torch.position = {0.0f, 0.0f, 2.0f};
    torch.direction = {0.0f, 0.0f, -1.0f};
    torch.innerCutoffDegrees = 10.0f;
    torch.outerCutoffDegrees = 20.0f;
    const auto dark = Fixture::off<day5::DirLight>();
    compare(f, dark, {}, 0, torch, true, "spotlight");
    const float center = f.target.at(0.15f, 0.0f).r;
    const float edge = f.target.at(0.55f, 0.0f).r;  // between the inner (0.35) and outer (0.73) radius
    const float outside = f.target.at(0.9f, 0.0f).r;
    CHECK_MSG(center > edge && edge > outside + 0.02f && outside < kPixelEps,
              "expected bright inside, dimmer at the edge, black outside. Red values: " << center << ", " << edge << ", " << outside);

    f.draw(dark, {}, 0, torch, false);
    CHECK_MSG(test::near(glm::vec3(f.target.at(0.15f, 0.0f)), glm::vec3(0.0f), kPixelEps),
              "with spotLightOn = false the spotlight should add nothing");
}

TEST("all lights together match the formula") {
    Fixture f;
    day5::DirLight sun;
    sun.direction = {-0.2f, -1.0f, -0.3f};
    std::vector<day5::PointLight> lamps(4);
    const glm::vec3 pos[4] = {{0.7f, 0.2f, 2.0f}, {-0.6f, -0.3f, 1.0f}, {0.4f, -0.8f, 0.6f}, {-0.5f, 0.7f, 1.5f}};
    const glm::vec3 col[4] = {{1.0f, 0.3f, 0.3f}, {0.3f, 1.0f, 0.3f}, {0.3f, 0.3f, 1.0f}, {1.0f, 1.0f, 0.4f}};
    for (int i = 0; i < 4; ++i) {
        lamps[i].position = pos[i];
        lamps[i].diffuse = col[i] * 0.8f;
        lamps[i].specular = col[i];
    }
    day5::SpotLight torch;
    torch.position = {0.1f, 0.1f, 2.5f};
    torch.direction = {-0.05f, 0.0f, -1.0f};
    compare(f, sun, lamps, 4, torch, true, "full scene");
}
