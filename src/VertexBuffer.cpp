#include "VertexBuffer.h"
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <utility>

VertexBuffer::VertexBuffer(std::size_t sizeInBytes, const void* data, bool dynamic)
    : m_size(sizeInBytes), m_dynamic(dynamic)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    Release();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0u)), m_size(std::exchange(other.m_size, std::size_t{ 0 })),
      m_dynamic(other.m_dynamic)
{
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_id = std::exchange(other.m_id, 0u);
        m_size = std::exchange(other.m_size, std::size_t{ 0 });
        m_dynamic = other.m_dynamic;
    }
    return *this;
}

void VertexBuffer::Release()
{
    if (m_id != 0)
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, std::size_t size, std::size_t offset)
{
    if (offset + size > m_size)
    {
        spdlog::error("VertexBuffer::SetData out of range (offset {} + size {} > buffer size {})", offset, size,
                      m_size);
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

void VertexBuffer::Orphan()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_size), nullptr, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}
