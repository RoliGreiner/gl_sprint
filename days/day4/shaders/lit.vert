#version 330 core
// Day 4 lit vertex shader. See the comment on makeLitShader in lighting.hpp.
// TODO: pass the world-space position (FragPos) and the normal (via normalMatrix) to the fragment shader.
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
