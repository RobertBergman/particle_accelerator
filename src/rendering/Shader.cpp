#include "rendering/Shader.hpp"
#include "utils/Logger.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace pas::rendering {

Shader::Shader() = default;

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_program(other.m_program)
    , m_uniformCache(std::move(other.m_uniformCache))
{
    other.m_program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_program != 0) {
            glDeleteProgram(m_program);
        }
        m_program = other.m_program;
        m_uniformCache = std::move(other.m_uniformCache);
        other.m_program = 0;
    }
    return *this;
}

bool Shader::load(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0) return false;

    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return false;
    }

    bool success = linkProgram(vertex, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return success;
}

bool Shader::load(const std::string& vertexSource,
                  const std::string& geometrySource,
                  const std::string& fragmentSource) {
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertex == 0) return false;

    unsigned int geometry = compileShader(GL_GEOMETRY_SHADER, geometrySource);
    if (geometry == 0) {
        glDeleteShader(vertex);
        return false;
    }

    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragment == 0) {
        glDeleteShader(vertex);
        glDeleteShader(geometry);
        return false;
    }

    bool success = linkProgram(vertex, fragment, geometry);

    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);

    return success;
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    return load(vertexSource, fragmentSource);
}

void Shader::use() const {
    if (m_program != 0) {
        glUseProgram(m_program);
    }
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" :
                              (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "GEOMETRY";
        PAS_ERROR("Shader compilation failed ({}): {}", typeStr, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(unsigned int vertex, unsigned int fragment, unsigned int geometry) {
    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    if (geometry != 0) {
        glAttachShader(m_program, geometry);
    }
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);

    int success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        PAS_ERROR("Shader linking failed: {}", infoLog);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    return true;
}

int Shader::getUniformLocation(const std::string& name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1) {
        PAS_WARN("Uniform '{}' not found or not used", name);
    }
    m_uniformCache[name] = location;
    return location;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        PAS_ERROR("Failed to open shader file: {}", path);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace pas::rendering
