// Runs every TEST in one day's test file. Provided, don't edit.
//
//   ./build/day1_tests            run all checks
//   ./build/day1_tests wireframe  run only checks whose name contains "wireframe"
//   ./build/day1_tests --list     list the checks

#include "sprint/gl_test.hpp"
#include "sprint/test.hpp"

#include <glad/glad.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iomanip>
#include <string>
#include <unistd.h>
#include <vector>

extern const char* const sprint_day_title;

namespace sprint::test {
namespace {

struct TestCase {
    const char* name;
    void (*fn)();
};

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

std::vector<std::string> g_messages;
const char* volatile g_current = nullptr;

void writeStr(const char* s) {
    if (s) (void)!write(2, s, std::strlen(s));
}

// A crash in exercise code would otherwise end the run with no hint about where it happened.
void onCrash(int sig) {
    writeStr("\n  CRASH ");
    writeStr(g_current ? g_current : "(outside a check)");
    writeStr(sig == SIGSEGV   ? "\n        Segmentation fault: an invalid pointer, often a GL call with a wrong size, offset or a deleted object."
             : sig == SIGFPE  ? "\n        Arithmetic error, for example an integer division by zero."
             : sig == SIGABRT ? "\n        abort() was called, for example by a failed assert or an uncaught exception."
                              : "\n        The program crashed.");
    writeStr("\n        Run this test target in the debugger (CLion: Shift+F9) to see the line.\n");
    _exit(3);
}

std::string baseName(const char* path) {
    std::string s(path);
    const auto slash = s.find_last_of('/');
    return slash == std::string::npos ? s : s.substr(slash + 1);
}

void resetGlState() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    for (int unit = 0; unit < 4; ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
}

void drainGlErrors(bool report) {
    for (int guard = 0; guard < 16; ++guard) {
        const GLenum err = glGetError();
        if (err == GL_NO_ERROR) return;
        if (report) {
            g_messages.push_back(std::string("OpenGL error ") + glErrorName(err) +
                                 " was raised during this check. Call glGetError() after your GL calls to find which one.");
        }
    }
}

}  // namespace

void registerTest(const char* name, void (*fn)()) { registry().push_back({name, fn}); }

bool check(bool ok, const char* expression, const std::string& message, const char* file, int line) {
    if (ok) return true;
    std::string text = message.empty() ? std::string("CHECK(") + expression + ") failed" : message;
    g_messages.push_back(text + "   [" + baseName(file) + ":" + std::to_string(line) + "]");
    return false;
}

bool near(const glm::vec2& a, const glm::vec2& b, float eps) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps);
}
bool near(const glm::vec3& a, const glm::vec3& b, float eps) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}
bool near(const glm::vec4& a, const glm::vec4& b, float eps) {
    return near(glm::vec3(a), glm::vec3(b), eps) && near(a.w, b.w, eps);
}
bool near(const glm::mat3& a, const glm::mat3& b, float eps) {
    for (int c = 0; c < 3; ++c)
        if (!near(a[c], b[c], eps)) return false;
    return true;
}
bool near(const glm::mat4& a, const glm::mat4& b, float eps) {
    for (int c = 0; c < 4; ++c)
        if (!near(a[c], b[c], eps)) return false;
    return true;
}
bool finite(const glm::mat4& m) {
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (!std::isfinite(m[c][r])) return false;
    return true;
}

static std::string fmt(float f) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(3) << f;
    return os.str();
}
std::string str(const glm::vec2& v) { return "(" + fmt(v.x) + ", " + fmt(v.y) + ")"; }
std::string str(const glm::vec3& v) { return "(" + fmt(v.x) + ", " + fmt(v.y) + ", " + fmt(v.z) + ")"; }
std::string str(const glm::vec4& v) {
    return "(" + fmt(v.x) + ", " + fmt(v.y) + ", " + fmt(v.z) + ", " + fmt(v.w) + ")";
}
std::string str(const glm::mat3& m) {
    std::string s = "[rows";
    for (int r = 0; r < 3; ++r) s += " " + str(glm::vec3(m[0][r], m[1][r], m[2][r]));
    return s + "]";
}
std::string str(const glm::mat4& m) {
    std::string s = "[rows";
    for (int r = 0; r < 4; ++r) s += " " + str(glm::vec4(m[0][r], m[1][r], m[2][r], m[3][r]));
    return s + "]";
}

}  // namespace sprint::test

int main(int argc, char** argv) {
    using namespace sprint::test;
    const std::string filter = argc > 1 ? argv[1] : "";
    const bool color = isatty(1) || std::getenv("CLION_IDE") != nullptr;
    const char* green = color ? "\033[32m" : "";
    const char* red = color ? "\033[31m" : "";
    const char* dim = color ? "\033[2m" : "";
    const char* reset = color ? "\033[0m" : "";

    if (filter == "--list") {
        for (const auto& t : registry()) std::printf("%s\n", t.name);
        return 0;
    }

    for (int sig : {SIGSEGV, SIGFPE, SIGABRT, SIGBUS}) std::signal(sig, onCrash);
    sprint::HiddenContext context;
    if (!context.ok()) {
        std::printf("Could not create an OpenGL 3.3 context for the tests: %s\n", context.error().c_str());
        return 2;
    }

    std::printf("\n%s\n", sprint_day_title);
    std::printf("%sOpenGL %s | %s%s\n\n", dim, reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)), reset);

    int run = 0, passed = 0;
    for (const auto& t : registry()) {
        if (!filter.empty() && std::string(t.name).find(filter) == std::string::npos) continue;
        ++run;
        g_messages.clear();
        g_current = t.name;
        drainGlErrors(false);
        try {
            t.fn();
        } catch (const RequireFailed&) {
            // message already recorded
        } catch (const std::exception& e) {
            g_messages.push_back(std::string("Stopped by an exception: ") + e.what());
        } catch (...) {
            g_messages.push_back("Stopped by an unknown exception");
        }
        drainGlErrors(true);
        resetGlState();
        drainGlErrors(false);

        if (g_messages.empty()) {
            ++passed;
            std::printf("  %sPASS%s  %s\n", green, reset, t.name);
        } else {
            std::printf("  %sFAIL%s  %s\n", red, reset, t.name);
            for (const auto& m : g_messages) std::printf("        %s\n", m.c_str());
        }
        std::fflush(stdout);
    }

    std::printf("\n%d of %d checks passed.\n", passed, run);
    if (run > 0 && passed == run) {
        std::printf("%sAll automatic checks pass.%s Do the \"check by eye\" items in the plan, then this day is done.\n",
                    green, reset);
        return 0;
    }
    if (run == 0) {
        std::printf("No checks matched \"%s\". Use --list to see their names.\n", filter.c_str());
        return 1;
    }
    std::printf("Fix the first FAIL from the top. Later checks often depend on earlier ones.\n");
    return 1;
}
