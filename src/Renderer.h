#pragma once
#include <cstddef>
#include <filesystem>
#include <memory>
#include <glm/glm.hpp>
#include "Camera.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "core/ParticleSystem.h"
#include "core/ThreadPool.h"

class Renderer
{
public:
    struct Config
    {
        std::filesystem::path shaderDir;
        int width = 1280;
        int height = 720;
        std::size_t particleCapacity;
        std::size_t threads = 0;
    };

    explicit Renderer(const Config& config);

    void Update(float dt); 
    void Draw();           
    void OnResize(int width, int height);

    Camera& GetCamera() { return m_camera; }

    void SetThreadCount(std::size_t totalThreads);
    std::size_t GetThreadCount() const { return m_threadCount; }

    std::size_t GetActiveParticles() const { return m_particles.GetActiveCount(); }
    double GetUpdateMs() const { return m_updateMs; }

private:
    void EmitParticles(float dt);

    Shader m_cubeShader;
    Shader m_particleShader;

    VertexBuffer m_cubeVertices;
    IndexBuffer m_cubeIndices;
    VertexBuffer m_cubeInstances;
    VertexArray m_cubeVA;

    VertexBuffer m_quadVertices;
    IndexBuffer m_quadIndices;
    VertexBuffer m_particleInstances;
    VertexArray m_particleVA;

    Camera m_camera;
    ParticleSystem m_particles;
    std::unique_ptr<ThreadPool> m_pool;
    std::size_t m_threadCount = 1;

    glm::mat4 m_projection{ 1.0f };
    float m_rotation = 0.0f;
    float m_emitAccumulator = 0.0f;
    double m_updateMs = 0.0;
    bool m_warnedAboutTruncation = false;
};
