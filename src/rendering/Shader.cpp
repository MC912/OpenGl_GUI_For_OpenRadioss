#include "rendering/Shader.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = LoadShaderFromFile(vertexPath);
    std::string fragmentCode = LoadShaderFromFile(fragmentPath);
    
    unsigned int vertex = CompileShader(GL_VERTEX_SHADER, vertexCode);
    unsigned int fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);
    
    // Shader Program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertex);
    glAttachShader(m_ID, fragment);
    glLinkProgram(m_ID);
    CheckCompileErrors(m_ID, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

void Shader::Use() const {
    glUseProgram(m_ID);
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

std::string Shader::LoadShaderFromFile(const std::string& path) {
    std::string code;
    std::ifstream file;
    
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        code = stream.str();
    } catch (std::ifstream::failure& e) {
        LOG_ERROR("Failed to read shader file: {}", path);
        
        // Return default shader if file not found
        if (path.find(".vert") != std::string::npos) {
            return R"(
                #version 330 core
                layout (location = 0) in vec3 aPos;
                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;
                void main() {
                    gl_Position = projection * view * model * vec4(aPos, 1.0);
                }
            )";
        } else {
            return R"(
                #version 330 core
                out vec4 FragColor;
                uniform vec3 color;
                void main() {
                    FragColor = vec4(color, 1.0);
                }
            )";
        }
    }
    
    return code;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    CheckCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    return shader;
}

void Shader::CheckCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            LOG_ERROR("Shader compilation error ({}): {}", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            LOG_ERROR("Shader linking error: {}", infoLog);
        }
    }
}
