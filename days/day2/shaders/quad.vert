#version 330 core
// Day 2 vertex shader for the textured quad.
// TODO: read the position (location 0) and texture coordinate (location 1),
// pass the texture coordinate on to the fragment shader, scaled by a uvScale uniform.
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
