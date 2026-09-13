#pragma once

#include <cstddef>
#include <cstdint>

struct mygpu_command_buffer;

class Buffer;

namespace myrenderer
{
    class CommandBuffer
    {
    public:
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer &) = delete;
        CommandBuffer &operator=(const CommandBuffer &) = delete;

        CommandBuffer(const CommandBuffer &&) = delete;
        CommandBuffer &operator=(const CommandBuffer &&) = delete;

        bool Valid() const;

        bool IsEmpty() const;

        bool Reset();

        bool Clear(uint32_t color);

        bool DrawRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

        bool BindVertexBuffer(const Buffer &vertexBuffer);

        bool DrawTriangles(uint32_t vertexCount);

    private:
        friend class Renderer;

        explicit CommandBuffer(struct mygpu_command_buffer *CommandBuffer);

        struct mygpu_command_buffer *m_commandBuffer;
        const Buffer *m_vertexBuffer;
    };
} // namespace myrenderer
