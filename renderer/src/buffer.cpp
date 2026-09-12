#include "myrenderer/buffer.hpp"

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

} // namespace myrenderer