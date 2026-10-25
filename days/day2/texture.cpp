// Day 2: loading textures. Replace the TODO, then run day2_tests.
#include "day2/texture.hpp"

#include <stb_image.h>

namespace day2 {

GLuint loadTexture([[maybe_unused]] const std::string& path, [[maybe_unused]] GLint wrap) {
    // TODO: stbi_set_flip_vertically_on_load, stbi_load, glGenTextures, glTexImage2D,
    // glGenerateMipmap, glTexParameteri, stbi_image_free. Lessons 2.6 to 2.8.
    return 0;
}

}  // namespace day2
