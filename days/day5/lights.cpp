// Day 5: materials and lights. Replace the TODOs, write the two shader files, then run day5_tests.
#include "day5/lights.hpp"

#include "sprint/window.hpp"  // SPRINT_DAY_FILE

namespace day5 {

std::unique_ptr<day2::Shader> makeSceneShader() {
    return std::make_unique<day2::Shader>(SPRINT_DAY_FILE("shaders/scene.vert"), SPRINT_DAY_FILE("shaders/scene.frag"));
}

void applyMaterial([[maybe_unused]] const day2::Shader& shader, [[maybe_unused]] const Material& material) {
    // TODO: lesson 5.2 and 2.9
}

void uploadDirLight([[maybe_unused]] const day2::Shader& shader, [[maybe_unused]] const DirLight& light) {
    // TODO: "dirLight.direction", ...
}

void uploadPointLight([[maybe_unused]] const day2::Shader& shader, [[maybe_unused]] const PointLight& light,
                      [[maybe_unused]] int index) {
    // TODO: "pointLights[0].position", ... (lesson 5.4)
}

void uploadPointLightCount([[maybe_unused]] const day2::Shader& shader, [[maybe_unused]] int count) {
    // TODO
}

void uploadSpotLight([[maybe_unused]] const day2::Shader& shader, [[maybe_unused]] const SpotLight& light,
                     [[maybe_unused]] bool enabled) {
    // TODO: the cutoffs go to the shader as cosines (lesson 5.3)
}

}  // namespace day5
