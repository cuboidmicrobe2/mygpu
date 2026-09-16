#include "myrenderer/buffer.hpp"
#include "myrenderer/vertex.hpp"

extern "C"
{
#include "mygpu/buffer.h"
}

namespace myrenderer
{

    Buffer::Buffer(struct mygpu_buffer *buffer) : m_buffer(buffer) {}

    Buffer::~Buffer()
    {
        if (m_buffer != nullptr)
        {
            mygpu_buffer_destroy(m_buffer);
            m_buffer = nullptr;
        }
    }

    bool Buffer::Valid() const
    {
        return m_buffer != nullptr;
    }

    size_t Buffer::Size() const
    {
        if (m_buffer == nullptr)
        {
            return 0;
        }

        return mygpu_buffer_size(m_buffer);
    }

    bool Buffer::Write(size_t offset, const void *data, size_t size)
    {
        if (m_buffer == nullptr)
        {
            return false;
        }

        return mygpu_buffer_write(m_buffer, offset, data, size) == 0;
    }

    bool Buffer::Read(size_t offset, void *data, size_t size) const
    {
        if (m_buffer == nullptr)
        {
            return false;
        }

        return mygpu_buffer_read(m_buffer, offset, data, size) == 0;
    }

    bool Buffer::WriteVertices(const Vertex *vertices, size_t vertexCount)
    {
        if (m_buffer == nullptr || vertices == nullptr || vertexCount == 0)
        {
            return false;
        }

        if (vertexCount > SIZE_MAX / sizeof(Vertex))
        {
            return false;
        }

        return Write(0, vertices, vertexCount * sizeof(Vertex));
    }

    bool Buffer::WriteIndices(const uint32_t *indices, size_t indexCount)
    {
        if (m_buffer == nullptr || indices == nullptr || indexCount == 0)
        {
            return false;
        }

        if (indexCount > SIZE_MAX / sizeof(uint32_t))
        {
            return false;
        }

        return Write(0, indices, indexCount * sizeof(uint32_t));
    }

    uint32_t Buffer::Address() const
    {
        if (m_buffer == nullptr)
        {
            return 0;
        }

        return mygpu_buffer_address(m_buffer);
    }

} // namespace myrenderer