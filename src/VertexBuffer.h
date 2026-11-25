#pragma once
#include <cstddef>

class VertexBuffer
{
public:
    explicit VertexBuffer(std::size_t sizeInBytes, const void* data = nullptr, bool dynamic = true);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void Bind() const;
    static void Unbind();

    void SetData(const void* data, std::size_t size, std::size_t offset = 0);

    void Orphan();

    std::size_t GetSize() const { return m_size; }

private:
    void Release();

    unsigned int m_id = 0;
    std::size_t m_size = 0;
    bool m_dynamic = true;
};
