#pragma once
#include <cstddef>
#include "IndexBuffer.h"
#include "VertexBuffer.h"

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void Bind() const;
    static void Unbind();

    void AddAttribute(const VertexBuffer& buffer, unsigned int location, int componentCount, std::size_t stride,
                      std::size_t offset, unsigned int divisor = 0);

    void SetIndexBuffer(const IndexBuffer& indexBuffer);

private:
    void Release();

    unsigned int m_id = 0;
};
