#version 330 core
// Day 4 lit fragment shader. See the comment on makeLitShader in lighting.hpp for
// the exact formulas the tests expect (lesson 4.6).
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
