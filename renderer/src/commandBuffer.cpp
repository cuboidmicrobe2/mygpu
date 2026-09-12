#include "myrenderer/buffer.hpp"
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

    bool CommandBuffer::Clear(uint32_t color)
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        struct mygpu_cmd_clear command;

        command.opcode = MYGPU_CMD_CLEAR;
        command.color = color;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::DrawTriangles(const Buffer &vertexBuffer, uint32_t vertexCount)
    {
        if (m_commandBuffer == nullptr || !vertexBuffer.Valid())
        {
            return false;
        }

        struct mygpu_cmd_draw_triangles command;

        command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
        command.vertex_address = vertexBuffer.Address();
        command.vertex_count = vertexCount;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

} // namespace myrenderer
