// Day 1 exercise. Replace each TODO with your code, then run day1_tests.
// The comments in exercise.hpp say exactly what each function must do.
#include "day1/exercise.hpp"

namespace day1 {

GLuint compileShader([[maybe_unused]] GLenum type, [[maybe_unused]] const char* source,
                     [[maybe_unused]] std::string* log) {
    // DONE: glCreateShader, glShaderSource, glCompileShader, then check GL_COMPILE_STATUS.
    GLuint shader = glCreateShader(type);
    if (shader) {
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            if (log) {
                char info_log[512];
                glGetShaderInfoLog(shader, 512, nullptr, info_log);
                *log += info_log;
            }
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }
    return 0;
}

GLuint linkProgram([[maybe_unused]] GLuint vertexShader, [[maybe_unused]] GLuint fragmentShader,
                   [[maybe_unused]] std::string* log) {
    // DONE: glCreateProgram, attach both shaders, glLinkProgram, then check GL_LINK_STATUS.
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        if (log) {
            char info_log[512];
            glGetProgramInfoLog(program, 512, nullptr, info_log);
            *log += info_log;
        }
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

GLuint buildProgram([[maybe_unused]] const char* vertexSource, [[maybe_unused]] const char* fragmentSource,
                    [[maybe_unused]] std::string* log) {
    // DONE: use compileShader and linkProgram, and delete the shader objects afterwards.
    GLuint vertex_shader = compileShader(GL_VERTEX_SHADER, vertexSource, log);
    GLuint fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, log);

    if (!vertex_shader || !fragment_shader) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    GLuint program = linkProgram(vertex_shader, fragment_shader, log);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

GLuint makeOrangeProgram([[maybe_unused]] std::string* log) {
    // DONE: write the vertex and fragment shader sources and call buildProgram.
    const char *vertex_source = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
           gl_Position = vec4(aPos, 1.0);
        }
    )";
    const char *fragment_source = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
        }
    )";

    return buildProgram(vertex_source, fragment_source, log);
}

GLuint makeYellowProgram([[maybe_unused]] std::string* log) {
    // DONE
    const char *vertex_source = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
           gl_Position = vec4(aPos, 1.0);
        }
    )";
    const char *fragment_source = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
        }
    )";

    return buildProgram(vertex_source, fragment_source, log);
}

Mesh makeLeftTriangle() {
    // DONE: VAO + VBO, glBufferData, glVertexAttribPointer for location 0, glEnableVertexAttribArray.
    Mesh mesh{};
    float vertices[] = {
        -0.9f, -0.5f, 0.0f,
        -0.1f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    mesh.count = 3;

    return mesh;
}

Mesh makeRightTriangle() {
    // DONE
    Mesh mesh{};
    float vertices[] = {
        0.1f, -0.5f, 0.0f,
        0.9f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    mesh.count = 3;

    return mesh;
}

Mesh makeRectangle() {
    // DONE: 4 vertices in the VBO, 6 indices in an EBO. Mind lesson 1.8 about the EBO and the VAO.
    Mesh mesh{};
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,   // first triangle
        0, 2, 3    // second triangle
    };

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    mesh.count = 6;
    mesh.indexed = true;

    return mesh;
}

void drawMesh([[maybe_unused]] const Mesh& mesh) {
    // DONE
    glBindVertexArray(mesh.vao);
    if (mesh.indexed) {
        glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh.count);
    }
}

void destroyMesh([[maybe_unused]] Mesh& mesh) {
    // DONE
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
    mesh = {};
}

void setWireframe([[maybe_unused]] bool enabled) {
    // DONE: glPolygonMode
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void onFramebufferResize([[maybe_unused]] int width, [[maybe_unused]] int height) {
    // DONE
    glViewport(0, 0, width, height);
}

}  // namespace day1
