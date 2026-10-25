// Day 2: Shader class. Replace the TODOs, then run day2_tests.
#include "day2/shader.hpp"

namespace day2 {

Shader::Shader([[maybe_unused]] const std::string& vertexPath, [[maybe_unused]] const std::string& fragmentPath) {
    // TODO: read both files (std::ifstream), compile both stages, link, delete the shader objects.
    // On failure, leave id_ = 0 and fill error_.
    error_ = "Shader is not implemented yet";
}

Shader::~Shader() {
    // TODO
}

bool Shader::valid() const { return id_ != 0; }
const std::string& Shader::error() const { return error_; }
GLuint Shader::id() const { return id_; }

void Shader::use() const {
    // TODO
}

void Shader::setInt([[maybe_unused]] const std::string& name, [[maybe_unused]] int value) const {
    // TODO: glGetUniformLocation, then glUniform1i
}

void Shader::setFloat([[maybe_unused]] const std::string& name, [[maybe_unused]] float value) const {
    // TODO
}

void Shader::setVec3([[maybe_unused]] const std::string& name, [[maybe_unused]] const glm::vec3& value) const {
    // TODO
}

void Shader::setMat3([[maybe_unused]] const std::string& name, [[maybe_unused]] const glm::mat3& value) const {
    // TODO
}

void Shader::setMat4([[maybe_unused]] const std::string& name, [[maybe_unused]] const glm::mat4& value) const {
    // TODO: glUniformMatrix4fv with glm::value_ptr (lesson 3.6)
}

}  // namespace day2
