#pragma once
// Day 2: loading images into textures (lessons 2.6 to 2.8).

#include <glad/glad.h>

#include <string>

namespace day2 {

// Loads an image file (PNG, JPG, ...) into a new 2D texture and returns it, or 0 if
// the file cannot be loaded. It must:
//  - flip the image so the first row in memory is the bottom of the picture
//  - choose GL_RED / GL_RG / GL_RGB / GL_RGBA from the channel count stb_image reports
//  - work for images whose width is not a multiple of 4
//  - set wrap mode `wrap` on S and T
//  - use GL_LINEAR_MIPMAP_LINEAR for minification, GL_LINEAR for magnification
//  - generate mipmaps
// stb_image is already compiled by the project: just #include <stb_image.h>,
// without defining STB_IMAGE_IMPLEMENTATION.
GLuint loadTexture(const std::string& path, GLint wrap = GL_REPEAT);

}  // namespace day2
