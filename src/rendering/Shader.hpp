#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief OpenGL shader program wrapper.
 *
 * Handles shader compilation, linking, and uniform management.
 */
class Shader {
public:
    Shader();
    ~Shader();

    // Non-copyable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Movable
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /**
     * @brief Load shader from source strings.
     * @param vertexSource Vertex shader source code.
     * @param fragmentSource Fragment shader source code.
     * @return True if compilation and linking succeeded.
     */
    bool load(const std::string& vertexSource, const std::string& fragmentSource);

    /**
     * @brief Load shader with geometry shader.
     * @param vertexSource Vertex shader source code.
     * @param geometrySource Geometry shader source code.
     * @param fragmentSource Fragment shader source code.
     * @return True if compilation and linking succeeded.
     */
    bool load(const std::string& vertexSource,
              const std::string& geometrySource,
              const std::string& fragmentSource);

    /**
     * @brief Load shaders from files.
     * @param vertexPath Path to vertex shader file.
     * @param fragmentPath Path to fragment shader file.
     * @return True if loading, compilation, and linking succeeded.
     */
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief Activate this shader program.
     */
    void use() const;

    /**
     * @brief Check if shader is valid and loaded.
     */
    bool isValid() const { return m_program != 0; }

    /**
     * @brief Get the OpenGL program ID.
     */
    unsigned int getProgram() const { return m_program; }

    // Uniform setters
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    unsigned int compileShader(unsigned int type, const std::string& source);
    bool linkProgram(unsigned int vertex, unsigned int fragment, unsigned int geometry = 0);
    int getUniformLocation(const std::string& name);
    std::string readFile(const std::string& path);

    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformCache;
};

} // namespace pas::rendering
