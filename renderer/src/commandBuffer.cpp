#include "myrenderer/commandBuffer.hpp"

extern "C"
{
#include "mygpu/commands.h"
}

namespace myrenderer
{
    CommandBuffer::CommandBuffer(struct mygpu_command_buffer *CommandBuffer) : m_commandBuffer(CommandBuffer) {}

    CommandBuffer::~CommandBuffer()
    {
        if (m_commandBuffer != nullptr)
        {
            mygpu_command_buffer_destroy(m_commandBuffer);
            m_commandBuffer = nullptr;
        }
    }

    bool CommandBuffer::Valid() const
    {
        return m_commandBuffer != nullptr;
    }

} // namespace myrenderer
