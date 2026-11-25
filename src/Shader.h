#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;

    void SetMat4(const std::string& name, const glm::mat4& value);
    void SetVec3(const std::string& name, const glm::vec3& value);

private:
    int GetUniformLocation(const std::string& name);
    void Release();

    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformCache;
};
