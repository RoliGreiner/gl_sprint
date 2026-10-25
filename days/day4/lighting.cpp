// Day 4: normals and lighting. Replace the TODO, write the four shader files, then run day4_tests.
#include "day4/lighting.hpp"

#include "sprint/window.hpp"  // SPRINT_DAY_FILE

namespace day4 {

glm::mat3 normalMatrix(const glm::mat4& model) {
    // TODO: lesson 4.5. This is wrong for non-uniform scale:
    return glm::mat3(model);
}

std::unique_ptr<day2::Shader> makeLitShader() {
    return std::make_unique<day2::Shader>(SPRINT_DAY_FILE("shaders/lit.vert"), SPRINT_DAY_FILE("shaders/lit.frag"));
}

std::unique_ptr<day2::Shader> makeLightCubeShader() {
    return std::make_unique<day2::Shader>(SPRINT_DAY_FILE("shaders/light.vert"), SPRINT_DAY_FILE("shaders/light.frag"));
}

}  // namespace day4
