#include "ParticleSystem.h"
#include "Random.h"
#include <algorithm>

ParticleSystem::ParticleSystem(std::size_t capacity)
    : m_inActive(capacity), m_Active(capacity), m_poolIndex(capacity > 0 ? capacity - 1 : 0)
{
}

void ParticleSystem::Emit(const ParticleProps& props)
{
    if (m_inActive.empty())
        return;

    InActive& inActive = m_inActive[m_poolIndex];
    Active& active = m_Active[m_poolIndex];

    const float lifeSpan = std::max(props.lifeSpan, 1e-4f);

    inActive.position = props.position;
    inActive.velocity = props.velocity;
    inActive.velocity.x += props.velocityVariation.x * (Random::Float() - 0.5f);
    inActive.velocity.y += props.velocityVariation.y * (Random::Float() - 0.5f);
    inActive.lifeRemaining = lifeSpan;

    active.colorBegin = props.colorBegin;
    active.colorEnd = props.colorEnd;
    active.sizeBegin = props.sizeBegin + props.sizeVariation * (Random::Float() - 0.5f);
    active.sizeEnd = props.sizeEnd;
    active.lifeSpan = lifeSpan;

    m_poolIndex = (m_poolIndex == 0) ? m_inActive.size() - 1 : m_poolIndex - 1;
}

void ParticleSystem::Update(float dt, ThreadPool& pool)
{
    m_chunks.resize(pool.GetChunkCount());
    for (auto& chunk : m_chunks)
        chunk.data.clear();

    pool.ParallelFor(
        m_inActive.size(),
        [this, dt](std::size_t begin, std::size_t end, std::size_t chunkIndex)
        {
            std::vector<ParticleInstance>& out = m_chunks[chunkIndex].data;

            for (std::size_t i = begin; i < end; ++i)
            {
                InActive& inActive = m_inActive[i];
                if (inActive.lifeRemaining <= 0.0f)
                    continue;

                inActive.lifeRemaining -= dt;
                if (inActive.lifeRemaining <= 0.0f)
                    continue; 

                inActive.position += inActive.velocity * dt;

                const Active& active = m_Active[i];
                const float t = 1.0f - inActive.lifeRemaining / active.lifeSpan; 

                out.push_back({inActive.position, glm::mix(active.colorBegin, active.colorEnd, t),
                               glm::mix(active.sizeBegin, active.sizeEnd, t)});
            }
        });

    m_activeCount = 0;
    for (const auto& chunk : m_chunks)
        m_activeCount += chunk.data.size();
}

std::vector<ParticleInstance> ParticleSystem::Collect() const
{
    std::vector<ParticleInstance> all;
    all.reserve(m_activeCount);
    for (const auto& chunk : m_chunks)
        all.insert(all.end(), chunk.data.begin(), chunk.data.end());
    return all;
}