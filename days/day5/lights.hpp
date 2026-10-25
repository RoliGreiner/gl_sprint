#pragma once
// Day 5: materials and light casters (lessons 5.1 to 5.4).

#include "day2/shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

namespace day5 {

struct Material {
    GLuint diffuseMap = 0;   // texture: surface color
    GLuint specularMap = 0;  // texture: how shiny each point is
    float shininess = 32.0f;
};

struct DirLight {
    glm::vec3 direction{-0.2f, -1.0f, -0.3f};  // the way the light travels, from the light into the scene
    glm::vec3 ambient{0.05f};
    glm::vec3 diffuse{0.4f};
    glm::vec3 specular{0.5f};
};

struct PointLight {
    glm::vec3 position{0.0f};
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    glm::vec3 ambient{0.05f};
    glm::vec3 diffuse{0.8f};
    glm::vec3 specular{1.0f};
};

struct SpotLight {
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    float innerCutoffDegrees = 12.5f;  // full brightness inside this angle
    float outerCutoffDegrees = 15.0f;  // dark outside this angle
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    glm::vec3 ambient{0.0f};
    glm::vec3 diffuse{1.0f};
    glm::vec3 specular{1.0f};
};

inline constexpr int kMaxPointLights = 4;

// Loads SPRINT_DAY_FILE("shaders/scene.vert") and ("shaders/scene.frag").
//
// Vertex inputs: location 0 vec3 position, 1 vec3 normal, 2 vec2 texture coordinate.
// Uniforms:
//   mat4 model, view, projection; mat3 normalMatrix; vec3 viewPos;
//   struct Material { sampler2D diffuse; sampler2D specular; float shininess; } material;
//   struct DirLight { vec3 direction; vec3 ambient; vec3 diffuse; vec3 specular; } dirLight;
//   struct PointLight { vec3 position; float constant; float linear; float quadratic;
//                       vec3 ambient; vec3 diffuse; vec3 specular; } pointLights[4];
//   int numPointLights;   // only the first numPointLights entries are used
//   struct SpotLight { vec3 position; vec3 direction; float cutOff; float outerCutOff;
//                      float constant; float linear; float quadratic;
//                      vec3 ambient; vec3 diffuse; vec3 specular; } spotLight;
//   bool spotLightOn;
//
// Formulas, like LearnOpenGL's "Multiple lights" chapter, with D = diffuse map color,
// S = specular map color (rgb), N = normalized normal, V = normalized direction to viewPos:
//   for a light direction L:  diff = max(dot(N, L), 0)
//                             spec = pow(max(dot(V, reflect(-L, N)), 0), material.shininess)
//                             light = ambient * D + diffuse * diff * D + specular * spec * S
//   directional: L = normalize(-direction), no falloff
//   point:       L = normalize(position - fragPos), all three parts times
//                attenuation = 1 / (constant + linear * d + quadratic * d * d)
//   spot:        like point, and all three parts also times
//                intensity = clamp((theta - outerCutOff) / (cutOff - outerCutOff), 0, 1)
//                with theta = dot(L, normalize(-direction))
//   color = directional + every used point light + spot (when spotLightOn), alpha 1.
std::unique_ptr<day2::Shader> makeSceneShader();

// Binds diffuseMap to texture unit 0 and specularMap to unit 1, and sets
// material.diffuse = 0, material.specular = 1 and material.shininess.
void applyMaterial(const day2::Shader& shader, const Material& material);

// Set the uniforms of one light. The shader must be in use.
void uploadDirLight(const day2::Shader& shader, const DirLight& light);
void uploadPointLight(const day2::Shader& shader, const PointLight& light, int index);  // "pointLights[index]...."
void uploadPointLightCount(const day2::Shader& shader, int count);                     // "numPointLights"
// cutOff and outerCutOff are uploaded as cosines of the angles (lesson 5.3).
void uploadSpotLight(const day2::Shader& shader, const SpotLight& light, bool enabled);

}  // namespace day5
