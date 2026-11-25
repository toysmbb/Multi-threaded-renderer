#include "VertexArray.h"
#include <glad/glad.h>
#include <utility>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    Release();
}

VertexArray::VertexArray(VertexArray&& other) noexcept : m_id(std::exchange(other.m_id, 0u)) {}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other)
    {
        Release();
        m_id = std::exchange(other.m_id, 0u);
    }
    return *this;
}

void VertexArray::Release()
{
    if (m_id != 0)
    {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::Unbind()
{
    glBindVertexArray(0);
}

void VertexArray::AddAttribute(const VertexBuffer& buffer, unsigned int location, int componentCount,
                               std::size_t stride, std::size_t offset, unsigned int divisor)
{
    glBindVertexArray(m_id);
    buffer.Bind();

    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, componentCount, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride),
                          reinterpret_cast<const void*>(offset));
    glVertexAttribDivisor(location, divisor);

    glBindVertexArray(0);
}

void VertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
    glBindVertexArray(m_id);
    indexBuffer.Bind(); 
    glBindVertexArray(0);
}
