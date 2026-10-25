// Automatic checks for Day 2. Provided, don't edit.
#include "day2/quad_scene.hpp"
#include "day2/shader.hpp"
#include "day2/texture.hpp"

#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <type_traits>
#include <utility>

SPRINT_DAY("Day 2: A textured quad you can control");

using namespace sprint;
using sprint::test::str;

namespace {

const char* kVS = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform float uScale;
uniform vec3 uOffset;
void main() { gl_Position = vec4(aPos * uScale + uOffset, 1.0); }
)";
const char* kFS = R"(#version 330 core
uniform int uMode;
out vec4 c;
void main() { c = vec4(float(uMode), 0.0, 0.0, 1.0); }
)";
const char* kBrokenFS = R"(#version 330 core
out vec4 c;
void main() { c = vec4(1.0) }
)";
const char* kNoMainFS = R"(#version 330 core
out vec4 c;
void notMain() { c = vec4(1.0); }
)";

struct ShaderFiles {
    std::string dir = tempDir();
    std::string vert = dir + "/test.vert";
    std::string frag = dir + "/test.frag";
    ShaderFiles(const char* vs, const char* fs) {
        writeFile(vert, vs);
        writeFile(frag, fs);
    }
};

bool isColor(const glm::vec4& got, const glm::vec4& want, float eps = kPixelEps) { return test::near(got, want, eps); }

const glm::vec4 kRed{1, 0, 0, 1}, kGreen{0, 1, 0, 1}, kBlue{0, 0, 1, 1};

template <class S>
void checkMoveSemantics(const ShaderFiles& files) {
    CHECK_MSG(!std::is_copy_constructible_v<S>,
              "Shader can be copied. A copy would delete the same program twice. Delete the copy constructor");
    CHECK_MSG(!std::is_copy_assignable_v<S>, "Shader can be copy-assigned. Delete the copy assignment operator");
    if constexpr (std::is_move_constructible_v<S> && !std::is_copy_constructible_v<S>) {
        S a(files.vert, files.frag);
        const GLuint id = a.id();
        REQUIRE(id != 0);
        {
            S b(std::move(a));
            CHECK_MSG(b.id() == id, "the moved-to Shader should own the original program");
            CHECK_MSG(a.id() == 0, "the moved-from Shader should have id() == 0, it has " << a.id());
        }
        CHECK_MSG(!glIsProgram(id), "destroying the moved-to Shader should delete the program");
    } else {
        CHECK_MSG(false, "Shader should be movable (move constructor), so it can be returned from functions");
    }
    if constexpr (std::is_move_assignable_v<S> && !std::is_copy_assignable_v<S>) {
        S a(files.vert, files.frag);
        S b(files.vert, files.frag);
        const GLuint oldB = b.id(), idA = a.id();
        REQUIRE(oldB != 0 && idA != 0);
        b = std::move(a);
        CHECK_MSG(b.id() == idA, "after b = std::move(a), b should own a's program");
        CHECK_MSG(!glIsProgram(oldB), "move assignment should delete the program b owned before");
        CHECK_MSG(a.id() == 0, "the moved-from Shader should have id() == 0");
    } else {
        CHECK_MSG(false, "Shader should be move-assignable");
    }
}

std::vector<unsigned char> rgbBytes(const std::vector<glm::ivec3>& px) {
    std::vector<unsigned char> b;
    for (auto p : px) b.insert(b.end(), {(unsigned char)p.r, (unsigned char)p.g, (unsigned char)p.b});
    return b;
}

GLint texParam(GLuint tex, GLenum pname) {
    GLint v = 0;
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexParameteriv(GL_TEXTURE_2D, pname, &v);
    glBindTexture(GL_TEXTURE_2D, 0);
    return v;
}
GLint levelParam(GLuint tex, int level, GLenum pname) {
    GLint v = 0;
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, pname, &v);
    glBindTexture(GL_TEXTURE_2D, 0);
    return v;
}

day2::QuadScene requireScene() {
    day2::QuadScene scene = day2::createQuadScene();
    REQUIRE_MSG(scene.shader != nullptr, "createQuadScene left scene.shader empty");
    REQUIRE_MSG(scene.shader->valid(), "the quad shader did not build: " << scene.shader->error());
    REQUIRE_MSG(scene.vao != 0 && glIsVertexArray(scene.vao), "createQuadScene did not create a VAO");
    return scene;
}

}  // namespace

// ---------------------------------------------------------------------------
// Shader class
// ---------------------------------------------------------------------------
TEST("Shader builds a program from two files") {
    ShaderFiles files(kVS, kFS);
    day2::Shader s(files.vert, files.frag);
    REQUIRE_MSG(s.valid(), "valid() is false for two correct files. error(): " << s.error());
    CHECK_MSG(glIsProgram(s.id()), "id() is not a program object");
    GLint linked = 0;
    glGetProgramiv(s.id(), GL_LINK_STATUS, &linked);
    CHECK_MSG(linked == GL_TRUE, "the program is not linked");
}

TEST("Shader reports a missing file with its path") {
    ShaderFiles files(kVS, kFS);
    const std::string missing = files.dir + "/does_not_exist.frag";
    day2::Shader s(files.vert, missing);
    CHECK_MSG(!s.valid(), "valid() should be false when a file is missing");
    CHECK_MSG(s.error().find(missing) != std::string::npos,
              "error() should contain the missing path \"" << missing << "\", it is: \"" << s.error() << "\"");
}

TEST("Shader reports a compile error with the file path and the driver log") {
    ShaderFiles files(kVS, kBrokenFS);
    day2::Shader s(files.vert, files.frag);
    CHECK_MSG(!s.valid(), "valid() should be false for a fragment shader with a syntax error");
    CHECK_MSG(s.error().find(files.frag) != std::string::npos,
              "error() should name the broken file \"" << files.frag << "\", it is: \"" << s.error() << "\"");
    CHECK_MSG(s.error().size() > files.frag.size() + 10, "error() should include the driver's info log, it is: \"" << s.error() << "\"");
}

TEST("Shader reports a link error") {
    {
        ShaderFiles good(kVS, kFS);
        day2::Shader ok(good.vert, good.frag);
        REQUIRE_MSG(ok.valid(), "a correct pair must build first. error(): " << ok.error());
    }
    ShaderFiles files(kVS, kNoMainFS);
    day2::Shader s(files.vert, files.frag);
    CHECK_MSG(!s.valid(), "valid() should be false when linking fails (fragment shader has no main)");
    CHECK_MSG(!s.error().empty(), "error() should contain the link log");
}

TEST("Shader setInt, setFloat and setVec3 set the uniforms") {
    ShaderFiles files(kVS, kFS);
    day2::Shader s(files.vert, files.frag);
    REQUIRE_MSG(s.valid(), s.error());
    s.use();
    GLint current = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    CHECK_MSG(static_cast<GLuint>(current) == s.id(), "use() should make this program current");
    s.setInt("uMode", 7);
    s.setFloat("uScale", 0.25f);
    s.setVec3("uOffset", {1.5f, -2.0f, 3.25f});
    CHECK_MSG(uniformInt(s.id(), "uMode") == 7, "setInt: uMode is " << uniformInt(s.id(), "uMode") << ", expected 7");
    CHECK_MSG(test::near(uniformFloat(s.id(), "uScale"), 0.25f), "setFloat: uScale is " << uniformFloat(s.id(), "uScale"));
    CHECK_MSG(test::near(uniformVec3(s.id(), "uOffset"), {1.5f, -2.0f, 3.25f}),
              "setVec3: uOffset is " << str(uniformVec3(s.id(), "uOffset")));
    glUseProgram(0);
}

TEST("Shader deletes its program when destroyed") {
    ShaderFiles files(kVS, kFS);
    GLuint id = 0;
    {
        day2::Shader s(files.vert, files.frag);
        REQUIRE_MSG(s.valid(), s.error());
        id = s.id();
    }
    CHECK_MSG(!glIsProgram(id), "the program still exists after the Shader went out of scope");
}

TEST("Shader cannot be copied but can be moved") {
    ShaderFiles files(kVS, kFS);
    checkMoveSemantics<day2::Shader>(files);
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------
TEST("loadTexture returns 0 for a missing file") {
    const std::string dir = tempDir();
    writePng(dir + "/ok.png", 2, 2, 3, std::vector<unsigned char>(12, 90));
    GLuint ok = day2::loadTexture(dir + "/ok.png");
    REQUIRE_MSG(ok != 0, "loadTexture must load a valid PNG first");
    glDeleteTextures(1, &ok);
    CHECK_MSG(day2::loadTexture(dir + "/nope.png") == 0, "a missing file should give 0");
}

TEST("loadTexture flips the image and handles a width that is not a multiple of 4") {
    // 3 x 3 RGB image, top row first as in the file. 9 bytes per row, so the default
    // 4-byte row alignment would read it wrongly.
    const std::vector<glm::ivec3> top = {{255, 0, 0}, {200, 0, 0}, {150, 0, 0}};
    const std::vector<glm::ivec3> mid = {{0, 255, 0}, {0, 200, 0}, {0, 150, 0}};
    const std::vector<glm::ivec3> bot = {{0, 0, 255}, {0, 0, 200}, {0, 0, 150}};
    std::vector<glm::ivec3> all = top;
    all.insert(all.end(), mid.begin(), mid.end());
    all.insert(all.end(), bot.begin(), bot.end());
    const std::string path = tempDir() + "/rgb3x3.png";
    writePng(path, 3, 3, 3, rgbBytes(all));

    GLuint tex = day2::loadTexture(path);
    REQUIRE_MSG(tex != 0 && glIsTexture(tex), "loadTexture returned 0 for a valid PNG");
    CHECK_MSG(levelParam(tex, 0, GL_TEXTURE_WIDTH) == 3 && levelParam(tex, 0, GL_TEXTURE_HEIGHT) == 3,
              "texture should be 3x3");

    std::vector<unsigned char> got(27, 0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, got.data());
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    // In OpenGL row 0 is the bottom, so it must hold the file's last row.
    const std::vector<std::vector<glm::ivec3>> expected = {bot, mid, top};
    bool flipped = true, exact = true;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col) {
            const unsigned char* p = &got[(row * 3 + col) * 3];
            const glm::ivec3 want = expected[row][col];
            if (glm::ivec3(p[0], p[1], p[2]) != want) exact = false;
        }
    if (!exact) {
        const unsigned char* p = &got[0];
        flipped = glm::ivec3(p[0], p[1], p[2]) == bot[0];
    }
    CHECK_MSG(flipped, "the bottom row of the texture is not the bottom of the image. Call stbi_set_flip_vertically_on_load(true)");
    CHECK_MSG(exact || !flipped,
              "pixels are shifted between rows. For RGB images with a width that is not a multiple of 4, call "
              "glPixelStorei(GL_UNPACK_ALIGNMENT, 1) before glTexImage2D");
    glDeleteTextures(1, &tex);
}

TEST("loadTexture picks the format from the channel count") {
    const std::string dir = tempDir();
    writePng(dir + "/rgb.png", 4, 4, 3, std::vector<unsigned char>(4 * 4 * 3, 128));
    writePng(dir + "/rgba.png", 4, 4, 4, std::vector<unsigned char>(4 * 4 * 4, 128));
    GLuint rgb = day2::loadTexture(dir + "/rgb.png");
    GLuint rgba = day2::loadTexture(dir + "/rgba.png");
    REQUIRE_MSG(rgb && rgba, "loadTexture failed on a plain PNG");
    CHECK_MSG(levelParam(rgb, 0, GL_TEXTURE_ALPHA_SIZE) == 0, "an RGB image should give a texture without alpha (GL_RGB)");
    CHECK_MSG(levelParam(rgba, 0, GL_TEXTURE_ALPHA_SIZE) > 0, "an RGBA image should give a texture with alpha (GL_RGBA)");
    glDeleteTextures(1, &rgb);
    glDeleteTextures(1, &rgba);
}

TEST("loadTexture sets filtering, wrapping and builds mipmaps") {
    const std::string path = tempDir() + "/img.png";
    writePng(path, 8, 4, 4, std::vector<unsigned char>(8 * 4 * 4, 200));
    GLuint tex = day2::loadTexture(path, GL_CLAMP_TO_EDGE);
    REQUIRE(tex != 0);
    CHECK_MSG(texParam(tex, GL_TEXTURE_MIN_FILTER) == GL_LINEAR_MIPMAP_LINEAR, "min filter should be GL_LINEAR_MIPMAP_LINEAR");
    CHECK_MSG(texParam(tex, GL_TEXTURE_MAG_FILTER) == GL_LINEAR, "mag filter should be GL_LINEAR");
    CHECK_MSG(texParam(tex, GL_TEXTURE_WRAP_S) == GL_CLAMP_TO_EDGE && texParam(tex, GL_TEXTURE_WRAP_T) == GL_CLAMP_TO_EDGE,
              "the wrap argument should be used for S and T");
    CHECK_MSG(levelParam(tex, 1, GL_TEXTURE_WIDTH) == 4 && levelParam(tex, 3, GL_TEXTURE_WIDTH) == 1,
              "mipmap levels are missing. Call glGenerateMipmap after glTexImage2D");
    glDeleteTextures(1, &tex);

    GLuint repeat = day2::loadTexture(path);
    CHECK_MSG(texParam(repeat, GL_TEXTURE_WRAP_S) == GL_REPEAT, "the default wrap mode should be GL_REPEAT");
    glDeleteTextures(1, &repeat);
}

// ---------------------------------------------------------------------------
// The quad
// ---------------------------------------------------------------------------
TEST("quad covers the square from -0.5 to 0.5") {
    Offscreen target;
    day2::QuadScene scene = requireScene();
    GLuint red = makeSolidTexture(kRed), green = makeSolidTexture(kGreen);
    day2::drawQuadScene(scene, red, green, 0.0f);
    CHECK_MSG(isColor(target.at(0, 0), kRed), "center should show tex0 (red) at mix 0, got " << str(target.at(0, 0)));
    CHECK_MSG(isColor(target.at(0.45f, 0.45f), kRed) && isColor(target.at(-0.45f, -0.45f), kRed),
              "the quad does not reach its corners");
    CHECK_MSG(isColor(target.at(0.7f, 0.0f), kTestClearColor) && isColor(target.at(0.0f, -0.7f), kTestClearColor),
              "the quad is bigger than -0.5..0.5");
    glDeleteTextures(1, &red);
    glDeleteTextures(1, &green);
    day2::destroyQuadScene(scene);
}

TEST("mixAmount blends texture unit 0 and unit 1") {
    Offscreen target;
    day2::QuadScene scene = requireScene();
    GLuint red = makeSolidTexture(kRed), green = makeSolidTexture(kGreen);
    day2::drawQuadScene(scene, red, green, 1.0f);
    CHECK_MSG(isColor(target.at(0, 0), kGreen),
              "at mix 1 the quad should show tex1 (green), got " << str(target.at(0, 0))
                                                                 << ". Is the second sampler set to texture unit 1?");
    target.bind();
    day2::drawQuadScene(scene, red, green, 0.5f);
    CHECK_MSG(isColor(target.at(0, 0), {0.5f, 0.5f, 0.0f, 1.0f}, 0.02f),
              "at mix 0.5 expected (0.5, 0.5, 0), got " << str(target.at(0, 0)));
    target.bind();
    day2::drawQuadScene(scene, red, green, 0.0f);
    CHECK_MSG(isColor(target.at(0, 0), kRed), "at mix 0 expected red, got " << str(target.at(0, 0)));
    glDeleteTextures(1, &red);
    glDeleteTextures(1, &green);
    day2::destroyQuadScene(scene);
}

TEST("texture coordinates run from (0,0) bottom-left to (1,1) top-right") {
    Offscreen target;
    day2::QuadScene scene = requireScene();
    // 2 x 2, bottom row first: bottom-left red, bottom-right green, top-left blue, top-right white
    GLuint quadrants = makeTexture(2, 2, {kRed, kGreen, kBlue, glm::vec4(1)}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    GLuint black = makeSolidTexture({0, 0, 0, 1});
    day2::drawQuadScene(scene, quadrants, black, 0.0f);
    CHECK_MSG(isColor(target.at(-0.25f, -0.25f), kRed), "bottom-left should be red, got " << str(target.at(-0.25f, -0.25f)));
    CHECK_MSG(isColor(target.at(0.25f, -0.25f), kGreen), "bottom-right should be green, got " << str(target.at(0.25f, -0.25f)));
    CHECK_MSG(isColor(target.at(-0.25f, 0.25f), kBlue), "top-left should be blue, got " << str(target.at(-0.25f, 0.25f)));
    CHECK_MSG(isColor(target.at(0.25f, 0.25f), glm::vec4(1)), "top-right should be white, got " << str(target.at(0.25f, 0.25f)));
    glDeleteTextures(1, &quadrants);
    glDeleteTextures(1, &black);
    day2::destroyQuadScene(scene);
}

TEST("uvScale 2 with GL_REPEAT tiles the texture 2 x 2") {
    Offscreen target;
    day2::QuadScene scene = requireScene();
    GLuint halves = makeTexture(1, 2, {kBlue, kRed}, GL_NEAREST, GL_REPEAT);  // bottom blue, top red
    GLuint black = makeSolidTexture({0, 0, 0, 1});
    day2::drawQuadScene(scene, halves, black, 0.0f, 2.0f);
    const float ys[4] = {-0.4f, -0.15f, 0.1f, 0.35f};
    const glm::vec4 want[4] = {kBlue, kRed, kBlue, kRed};
    for (int i = 0; i < 4; ++i)
        CHECK_MSG(isColor(target.at(0.1f, ys[i]), want[i]),
                  "at y = " << ys[i] << " expected " << str(want[i]) << ", got " << str(target.at(0.1f, ys[i])));
    glDeleteTextures(1, &halves);
    glDeleteTextures(1, &black);
    day2::destroyQuadScene(scene);
}

TEST("destroyQuadScene deletes the GL objects") {
    day2::QuadScene scene = requireScene();
    const GLuint vao = scene.vao, vbo = scene.vbo, ebo = scene.ebo;
    day2::destroyQuadScene(scene);
    CHECK(!glIsVertexArray(vao));
    CHECK(!glIsBuffer(vbo));
    CHECK(!glIsBuffer(ebo));
}

// ---------------------------------------------------------------------------
// Arrow keys
// ---------------------------------------------------------------------------
TEST("updateMix changes by 1.0 per second and stays in 0..1") {
    CHECK_MSG(test::near(day2::updateMix(0.5f, true, false, 0.1f), 0.6f), "Up for 0.1 s from 0.5 should give 0.6");
    CHECK_MSG(test::near(day2::updateMix(0.5f, false, true, 0.25f), 0.25f), "Down for 0.25 s from 0.5 should give 0.25");
    CHECK_MSG(test::near(day2::updateMix(0.5f, true, true, 0.3f), 0.5f), "Up and Down together should cancel out");
    CHECK_MSG(test::near(day2::updateMix(0.5f, false, false, 0.3f), 0.5f), "no key should leave it unchanged");
    CHECK_MSG(test::near(day2::updateMix(0.95f, true, false, 0.5f), 1.0f), "must not go above 1");
    CHECK_MSG(test::near(day2::updateMix(0.05f, false, true, 0.5f), 0.0f), "must not go below 0");
}

TEST("updateMix gives the same result at 30 fps and at 300 fps") {
    float slow = 0.0f, fast = 0.0f;
    for (int i = 0; i < 15; ++i) slow = day2::updateMix(slow, true, false, 1.0f / 30.0f);
    for (int i = 0; i < 150; ++i) fast = day2::updateMix(fast, true, false, 1.0f / 300.0f);
    CHECK_MSG(test::near(slow, 0.5f, 1e-3f) && test::near(fast, 0.5f, 1e-3f),
              "after 0.5 s of holding Up from 0, expected 0.5 at any frame rate. Got " << slow << " at 30 fps and " << fast
                                                                                       << " at 300 fps");
}
