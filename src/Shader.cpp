#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error("Cannot open shader file: " + path.string());

        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

    unsigned int Compile(unsigned int type, const std::string& source, const std::filesystem::path& path)
    {
        const unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (ok == GL_FALSE)
        {
            char log[2048];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            glDeleteShader(shader);
            throw std::runtime_error("Shader compile error in " + path.string() + ":\n" + log);
        }
        return shader;
    }
}

Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
    const unsigned int vertex = Compile(GL_VERTEX_SHADER, ReadFile(vertexPath), vertexPath);

    unsigned int fragment = 0;
    try
    {
        fragment = Compile(GL_FRAGMENT_SHADER, ReadFile(fragmentPath), fragmentPath);
    }
    catch (...)
    {
        glDeleteShader(vertex);
        throw;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);

    glDetachShader(m_program, vertex);
    glDetachShader(m_program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        char log[2048];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        glDeleteProgram(m_program);
        m_program = 0;
        throw std::runtime_error(std::string("Shader link error:\n") + log);
    }
}

Shader::~Shader()
{
    Release();
}

Shader::Shader(Shader&& other) noexcept
    : m_program(std::exchange(other.m_program, 0u)), m_uniformCache(std::move(other.m_uniformCache))
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_program = std::exchange(other.m_program, 0u);
        m_uniformCache = std::move(other.m_uniformCache);
    }
    return *this;
}

void Shader::Release()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void Shader::Bind() const
{
    glUseProgram(m_program);
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (auto it = m_uniformCache.find(name); it != m_uniformCache.end())
        return it->second;

    const int location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1)
        spdlog::warn("Uniform '{}' not found (unused or misspelled)", name);

    m_uniformCache.emplace(name, location);
    return location;
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}
