#include "Renderer.h"
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Random.h"

namespace
{
    constexpr std::size_t kCubeCount = 100;                 
    constexpr std::size_t kMaxRenderedParticles = 262'144;  
    constexpr float kEmitPerSecond = 30'000.0f;           
    constexpr float kCubeRotationSpeed = 0.6f;        

    struct Vertex
    {
        float position[3];
        float normal[3];
        float uv[2];
    };

    constexpr Vertex kCubeVertices[] =
    {
        // +Z
        { {-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f} },
        // -Z
        { { 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f} },
        // -X 
        { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
        // +X
        { { 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
        // +Y
        { {-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },
        // -Y
        { {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} },
    };

    constexpr std::uint32_t kCubeIndices[] =
    {
         0,  1,  2,   2,  3,  0,
         4,  5,  6,   6,  7,  4,
         8,  9, 10,  10, 11,  8,
        12, 13, 14,  14, 15, 12,
        16, 17, 18,  18, 19, 16,
        20, 21, 22,  22, 23, 20,
    };

    constexpr float kQuadVertices[] =
    {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    constexpr std::uint32_t kQuadIndices[] = { 0, 1, 2, 2, 3, 0 };

    std::array<glm::vec3, kCubeCount> MakeCubeOffsets()
    {
        std::array<glm::vec3, kCubeCount> offsets{};
        std::size_t index = 0;
        for (int x = -10; x < 10; x += 2)
            for (int z = -10; z < 10; z += 2)
                offsets[index++] = glm::vec3(static_cast<float>(x), -1.0f, static_cast<float>(z));
        return offsets;
    }

    std::size_t ResolveThreadCount(std::size_t requested)
    {
        if (requested > 0)
            return requested;
        return std::max<std::size_t>(1, std::thread::hardware_concurrency());
    }
}

Renderer::Renderer(const Config& config)
    : m_cubeShader(config.shaderDir / "cube.vert", config.shaderDir / "cube.frag"),
      m_particleShader(config.shaderDir / "particle.vert", config.shaderDir / "particle.frag"),
      m_cubeVertices(sizeof(kCubeVertices), kCubeVertices, false),
      m_cubeIndices(kCubeIndices, std::size(kCubeIndices)),
      m_cubeInstances(sizeof(glm::vec3) * kCubeCount, MakeCubeOffsets().data(), false),
      m_quadVertices(sizeof(kQuadVertices), kQuadVertices, false),
      m_quadIndices(kQuadIndices, std::size(kQuadIndices)),
      m_particleInstances(sizeof(ParticleInstance) * kMaxRenderedParticles),
      m_camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      m_particles(config.particleCapacity)
{
    m_cubeVA.AddAttribute(m_cubeVertices, 0, 3, sizeof(Vertex), offsetof(Vertex, position));
    m_cubeVA.AddAttribute(m_cubeVertices, 1, 3, sizeof(Vertex), offsetof(Vertex, normal));
    m_cubeVA.AddAttribute(m_cubeVertices, 2, 2, sizeof(Vertex), offsetof(Vertex, uv));
    m_cubeVA.AddAttribute(m_cubeInstances, 3, 3, sizeof(glm::vec3), 0, 1);
    m_cubeVA.SetIndexBuffer(m_cubeIndices);

    m_particleVA.AddAttribute(m_quadVertices, 0, 3, 3 * sizeof(float), 0);
    m_particleVA.AddAttribute(m_particleInstances, 1, 2, sizeof(ParticleInstance), offsetof(ParticleInstance, position), 1);
    m_particleVA.AddAttribute(m_particleInstances, 2, 4, sizeof(ParticleInstance), offsetof(ParticleInstance, color), 1);
    m_particleVA.AddAttribute(m_particleInstances, 3, 1, sizeof(ParticleInstance), offsetof(ParticleInstance, size), 1);
    m_particleVA.SetIndexBuffer(m_quadIndices);

    glEnable(GL_DEPTH_TEST);

    SetThreadCount(ResolveThreadCount(config.threads));
    OnResize(config.width, config.height);
}

void Renderer::SetThreadCount(std::size_t totalThreads)
{
    m_threadCount = std::max<std::size_t>(1, totalThreads);
    m_pool = std::make_unique<ThreadPool>(m_threadCount - 1);
}

void Renderer::OnResize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return; 

    glViewport(0, 0, width, height);
    m_projection = m_camera.GetProjectionMatrix(static_cast<float>(width) / static_cast<float>(height));
}

void Renderer::EmitParticles(float dt)
{
    m_emitAccumulator += kEmitPerSecond * dt;
    const int count = static_cast<int>(m_emitAccumulator);
    m_emitAccumulator -= static_cast<float>(count);

    for (int i = 0; i < count; ++i)
    {
        ParticleProps props;
        props.velocity = { (Random::Float() - 0.5f) * 2.0f, (Random::Float() - 0.5f) * 2.0f };
        props.velocityVariation = { 0.5f, 0.5f };
        m_particles.Emit(props);
    }
}

void Renderer::Update(float dt)
{
    m_rotation += kCubeRotationSpeed * dt;
    EmitParticles(dt);

    const auto start = std::chrono::steady_clock::now();
    m_particles.Update(dt, *m_pool);
    const auto end = std::chrono::steady_clock::now();

    const double ms = std::chrono::duration<double, std::milli>(end - start).count();
    m_updateMs = (m_updateMs == 0.0) ? ms : m_updateMs * 0.95 + ms * 0.05; 
}

void Renderer::Draw()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 view = m_camera.GetViewMatrix();

    {
        const glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0.5f, 1.0f, 0.0f));

        m_cubeShader.Bind();
        m_cubeShader.SetMat4("u_Model", model);
        m_cubeShader.SetMat4("u_View", view);
        m_cubeShader.SetMat4("u_Projection", m_projection);

        m_cubeVA.Bind();
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_cubeIndices.GetCount()), GL_UNSIGNED_INT,
                                nullptr, static_cast<GLsizei>(kCubeCount));
    }

    std::size_t instanceCount = 0;
    {
        m_particleInstances.Orphan();

        std::size_t remaining = kMaxRenderedParticles;
        for (const auto& chunk : m_particles.GetChunks())
        {
            const std::size_t n = std::min(chunk.data.size(), remaining);
            if (n == 0)
                continue;

            m_particleInstances.SetData(chunk.data.data(), n * sizeof(ParticleInstance),
                                        instanceCount * sizeof(ParticleInstance));
            instanceCount += n;
            remaining -= n;
        }

        if (instanceCount < m_particles.GetActiveCount() && !m_warnedAboutTruncation)
        {
            spdlog::warn("{} particles are alive but only {} fit into the GPU buffer; the rest are not drawn",
                         m_particles.GetActiveCount(), kMaxRenderedParticles);
            m_warnedAboutTruncation = true;
        }
    }

    if (instanceCount > 0)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        m_particleShader.Bind();
        m_particleShader.SetMat4("u_View", view);
        m_particleShader.SetMat4("u_Projection", m_projection);

        m_particleVA.Bind();
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_quadIndices.GetCount()), GL_UNSIGNED_INT,
                                nullptr, static_cast<GLsizei>(instanceCount));

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    VertexArray::Unbind();
}
