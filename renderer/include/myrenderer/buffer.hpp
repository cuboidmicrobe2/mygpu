#pragma once

#include <cstddef>
#include <cstdint>

struct mygpu_buffer;

namespace myrenderer
{
    struct Vertex;

    class Buffer
    {
    public:
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        Buffer(const Buffer &&) = delete;
        Buffer &operator=(const Buffer &&) = delete;

        bool Valid() const;

        size_t Size() const;

        bool Write(size_t offset, const void *data, size_t size);

        bool Read(size_t offset, void *data, size_t size) const;

        bool WriteVertices(const Vertex *vertices, size_t vertexCount);

        bool WriteIndices(const uint32_t *indices, size_t indexCount);

        uint32_t Address() const;

    private:
        friend class Renderer;

        explicit Buffer(struct mygpu_buffer *buffer);

        struct mygpu_buffer *m_buffer;
    };
} // namespace myrenderer