#pragma once

#include <cstddef>

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

    private:
        friend class Renderer;

        explicit CommandBuffer(struct mygpu_command_buffer *CommandBuffer);

        struct mygpu_command_buffer *m_commandBuffer;
    };
} // namespace myrenderer
