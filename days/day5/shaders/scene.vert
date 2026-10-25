#version 330 core
// Day 5 scene vertex shader. See makeSceneShader in lights.hpp for what it needs.
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
