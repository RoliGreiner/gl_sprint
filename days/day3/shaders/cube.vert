#version 330 core
// Day 3 cube vertex shader.
// TODO: inputs at locations 0 (position) and 1 (texture coordinate),
// mat4 uniforms model, view, projection (lesson 3.3), pass the texture coordinate on.
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
