#pragma once
// A very small test framework for the sprint. Provided, don't edit.
//
//   TEST("what is being checked") { CHECK(a == b); REQUIRE(ptr != nullptr); }
//
// CHECK records a failure and continues. REQUIRE records a failure and stops that test.
// The *_MSG variants take a message built with <<, shown when the check fails.

#include <glm/glm.hpp>

#include <cmath>
#include <sstream>
#include <string>

namespace sprint::test {

struct RequireFailed {};

void registerTest(const char* name, void (*fn)());
bool check(bool ok, const char* expression, const std::string& message, const char* file, int line);

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registerTest(name, fn); }
};

inline bool near(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }
bool near(const glm::vec2& a, const glm::vec2& b, float eps = 1e-4f);
bool near(const glm::vec3& a, const glm::vec3& b, float eps = 1e-4f);
bool near(const glm::vec4& a, const glm::vec4& b, float eps = 1e-4f);
bool near(const glm::mat3& a, const glm::mat3& b, float eps = 1e-4f);
bool near(const glm::mat4& a, const glm::mat4& b, float eps = 1e-4f);
bool finite(const glm::mat4& m);

std::string str(const glm::vec2& v);
std::string str(const glm::vec3& v);
std::string str(const glm::vec4& v);
std::string str(const glm::mat3& m);
std::string str(const glm::mat4& m);

}  // namespace sprint::test

#define SPRINT_CAT_(a, b) a##b
#define SPRINT_CAT(a, b) SPRINT_CAT_(a, b)
#define SPRINT_TEST_(name, fn)                                                   \
    static void fn();                                                            \
    static const ::sprint::test::Registrar SPRINT_CAT(fn, _registrar){name, &fn}; \
    static void fn()
#define TEST(name) SPRINT_TEST_(name, SPRINT_CAT(sprint_test_, __LINE__))

#define CHECK(cond) ::sprint::test::check(static_cast<bool>(cond), #cond, std::string(), __FILE__, __LINE__)
#define CHECK_MSG(cond, msg)                                                                     \
    [&]() -> bool {                                                                              \
        const bool sprint_ok_ = static_cast<bool>(cond);                                         \
        if (sprint_ok_) return true;                                                             \
        std::ostringstream sprint_os_;                                                           \
        sprint_os_ << msg;                                                                       \
        return ::sprint::test::check(false, #cond, sprint_os_.str(), __FILE__, __LINE__);        \
    }()
#define REQUIRE(cond) \
    do { if (!CHECK(cond)) throw ::sprint::test::RequireFailed{}; } while (0)
#define REQUIRE_MSG(cond, msg) \
    do { if (!CHECK_MSG(cond, msg)) throw ::sprint::test::RequireFailed{}; } while (0)

// Put this once in each day's test file.
#define SPRINT_DAY(title) extern const char* const sprint_day_title; const char* const sprint_day_title = title
