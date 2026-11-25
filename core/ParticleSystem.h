#pragma once
#include <cstddef>
#include <vector>
#include <glm/glm.hpp>
#include "ThreadPool.h"

struct ParticleProps
{
    glm::vec2 position{0.0f, 0.0f};
    glm::vec2 velocity{0.0f, 0.0f};
    glm::vec2 velocityVariation{0.0f, 0.0f};
    glm::vec4 colorBegin{0.45f, 0.45f, 0.48f, 0.18f}; 
    glm::vec4 colorEnd{0.80f, 0.80f, 0.83f, 0.0f}; 
    float sizeBegin = 0.05f;
    float sizeEnd = 0.25f;
    float sizeVariation = 0.02f;
    float lifeSpan = 3.0f;
};

struct ParticleInstance
{
    glm::vec2 position;
    glm::vec4 color;
    float size;
};

class ParticleSystem
{
  public:
    struct alignas(64) Chunk
    {
        std::vector<ParticleInstance> data;
    };

    explicit ParticleSystem(std::size_t capacity);

    void Emit(const ParticleProps& props);

    void Update(float dt, ThreadPool& pool);

    std::size_t GetCapacity() const { return m_inActive.size(); }
    std::size_t GetActiveCount() const { return m_activeCount; }

    const std::vector<Chunk>& GetChunks() const { return m_chunks; }

    std::vector<ParticleInstance> Collect() const;

  private:
    struct InActive
    {
        glm::vec2 position{0.0f};
        glm::vec2 velocity{0.0f};
        float lifeRemaining = 0.0f;
    };

    struct Active
    {
        glm::vec4 colorBegin{0.0f};
        glm::vec4 colorEnd{0.0f};
        float sizeBegin = 0.0f;
        float sizeEnd = 0.0f;
        float lifeSpan = 1.0f;
    };

    std::vector<InActive> m_inActive;
    std::vector<Active> m_Active;
    std::vector<Chunk> m_chunks;
    std::size_t m_poolIndex;
    std::size_t m_activeCount = 0;
};