#include "IndexBuffer.h"
#include <glad/glad.h>
#include <utility>

IndexBuffer::IndexBuffer(const std::uint32_t* indices, std::size_t count) : m_count(count)
{
    glGenBuffers(1, &m_id);

    glBindBuffer(GL_COPY_WRITE_BUFFER, m_id);
    glBufferData(GL_COPY_WRITE_BUFFER, static_cast<GLsizeiptr>(count * sizeof(std::uint32_t)), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
    Release();
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0u)), m_count(std::exchange(other.m_count, std::size_t{ 0 }))
{
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_id = std::exchange(other.m_id, 0u);
        m_count = std::exchange(other.m_count, std::size_t{ 0 });
    }
    return *this;
}

void IndexBuffer::Release()
{
    if (m_id != 0)
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
