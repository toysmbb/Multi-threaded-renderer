#pragma once
#include <cstddef>
#include <cstdint>

class IndexBuffer
{
public:
    IndexBuffer(const std::uint32_t* indices, std::size_t count);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void Bind() const;

    std::size_t GetCount() const { return m_count; }

private:
    void Release();

    unsigned int m_id = 0;
    std::size_t m_count = 0;
};
