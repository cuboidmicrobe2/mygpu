#pragma once

#include <cstddef>
#include <cstdint>

struct mygpu_command_buffer;

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

        bool Clear(uint32_t color);

    private:
        friend class Renderer;

        explicit CommandBuffer(struct mygpu_command_buffer *CommandBuffer);

        struct mygpu_command_buffer *m_commandBuffer;
    };
} // namespace myrenderer
