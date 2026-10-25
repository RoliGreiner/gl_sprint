#pragma once
// Day 2: your Shader class (lesson 2.5). You will use it every day from now on.
//
// Keep the public functions as they are, the tests call them.
// You may add private members and helper functions.

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace day2 {

class Shader {
public:
    // Reads both files, compiles them and links the program. Never throws.
    // On any failure valid() is false and error() explains it:
    //  - a file that cannot be read: the message contains that file's path
    //  - a compile error: the message contains that file's path and the driver's info log
    //  - a link error: the message contains the info log
    Shader(const std::string& vertexPath, const std::string& fragmentPath);

    // Deletes the program.
    ~Shader();

    // TODO(lesson 2.5): decide what copying and moving a Shader should do, and declare it here.
    // The tests check that a Shader cannot be copied, can be moved, and that a moved-from
    // Shader no longer owns (or deletes) the program.

    bool valid() const;
    const std::string& error() const;
    GLuint id() const;

    void use() const;

    // Set a uniform of this program. Call use() first.
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;  // tested on Day 4
    void setMat4(const std::string& name, const glm::mat4& value) const;  // tested on Day 3

private:
    GLuint id_ = 0;
    std::string error_;
};

}  // namespace day2
