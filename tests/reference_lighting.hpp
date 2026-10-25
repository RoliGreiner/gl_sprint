#pragma once
// CPU versions of the lighting formulas, used by the Day 4 and Day 5 tests to know
// what color each pixel should have. Provided, don't edit.

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace reference {

inline glm::vec3 saturate(const glm::vec3& c) { return glm::clamp(c, glm::vec3(0.0f), glm::vec3(1.0f)); }

// Day 4
inline glm::vec3 phong(const glm::vec3& fragPos, const glm::vec3& normal, const glm::vec3& lightPos,
                       const glm::vec3& viewPos, const glm::vec3& lightColor, const glm::vec3& objectColor, bool blinn) {
    const glm::vec3 N = glm::normalize(normal);
    const glm::vec3 L = glm::normalize(lightPos - fragPos);
    const glm::vec3 V = glm::normalize(viewPos - fragPos);
    const glm::vec3 ambient = 0.1f * lightColor;
    const glm::vec3 diffuse = std::max(glm::dot(N, L), 0.0f) * lightColor;
    const float s = blinn ? std::pow(std::max(glm::dot(N, glm::normalize(L + V)), 0.0f), 32.0f)
                          : std::pow(std::max(glm::dot(V, glm::reflect(-L, N)), 0.0f), 32.0f);
    const glm::vec3 specular = 0.5f * s * lightColor;
    return saturate((ambient + diffuse + specular) * objectColor);
}

// Day 5
struct Surface {
    glm::vec3 pos, normal, diffuse, specular;  // diffuse/specular = texture colors at this point
    float shininess;
};
struct Dir {
    glm::vec3 direction, ambient, diffuse, specular;
};
struct Point {
    glm::vec3 position;
    float constant, linear, quadratic;
    glm::vec3 ambient, diffuse, specular;
};
struct Spot {
    glm::vec3 position, direction;
    float cutOff, outerCutOff;  // cosines
    float constant, linear, quadratic;
    glm::vec3 ambient, diffuse, specular;
};

inline glm::vec3 shade(const Surface& s, const glm::vec3& V, const glm::vec3& L, const glm::vec3& a,
                       const glm::vec3& d, const glm::vec3& sp, float scale) {
    const glm::vec3 N = glm::normalize(s.normal);
    const float diff = std::max(glm::dot(N, L), 0.0f);
    const float spec = std::pow(std::max(glm::dot(V, glm::reflect(-L, N)), 0.0f), s.shininess);
    return scale * (a * s.diffuse + d * diff * s.diffuse + sp * spec * s.specular);
}

inline glm::vec3 dirLight(const Dir& l, const Surface& s, const glm::vec3& V) {
    return shade(s, V, glm::normalize(-l.direction), l.ambient, l.diffuse, l.specular, 1.0f);
}

inline glm::vec3 pointLight(const Point& l, const Surface& s, const glm::vec3& V) {
    const float d = glm::length(l.position - s.pos);
    const float att = 1.0f / (l.constant + l.linear * d + l.quadratic * d * d);
    return shade(s, V, glm::normalize(l.position - s.pos), l.ambient, l.diffuse, l.specular, att);
}

inline glm::vec3 spotLight(const Spot& l, const Surface& s, const glm::vec3& V) {
    const glm::vec3 L = glm::normalize(l.position - s.pos);
    const float d = glm::length(l.position - s.pos);
    const float att = 1.0f / (l.constant + l.linear * d + l.quadratic * d * d);
    const float theta = glm::dot(L, glm::normalize(-l.direction));
    const float intensity = std::clamp((theta - l.outerCutOff) / (l.cutOff - l.outerCutOff), 0.0f, 1.0f);
    return shade(s, V, L, l.ambient, l.diffuse, l.specular, att * intensity);
}

}  // namespace reference
