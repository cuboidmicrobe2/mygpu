#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/vertex.hpp"

extern "C"
{
#include "mygpu/commands.h"
}

namespace myrenderer
{
    CommandBuffer::CommandBuffer(struct mygpu_command_buffer *CommandBuffer)
        : m_commandBuffer(CommandBuffer),
          m_vertexBuffer(nullptr),
          m_indexBuffer(nullptr) {}

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

    bool CommandBuffer::IsEmpty() const
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        return mygpu_command_buffer_is_empty(m_commandBuffer) == 1;
    }

    bool CommandBuffer::Reset()
    {
        if (m_commandBuffer == nullptr)
        {
            return false;
        }

        if (mygpu_command_buffer_reset(m_commandBuffer) != 0)
        {
            return false;
        }

        m_vertexBuffer = nullptr;
        m_indexBuffer = nullptr;

        return true;
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

    bool CommandBuffer::DrawRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color)
    {
        if (m_commandBuffer == nullptr || width == 0 || height == 0)
        {
            return false;
        }

        struct mygpu_cmd_draw_rect command;

        command.opcode = MYGPU_CMD_DRAW_RECT;
        command.x = x;
        command.y = y;
        command.width = width;
        command.height = height;
        command.color = color;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::BindVertexBuffer(const Buffer &vertexBuffer)
    {
        if (m_commandBuffer == nullptr || !vertexBuffer.Valid())
        {
            return false;
        }

        m_vertexBuffer = &vertexBuffer;

        return true;
    }

    bool CommandBuffer::DrawTriangles(uint32_t vertexCount)
    {
        if (m_commandBuffer == nullptr || m_vertexBuffer == nullptr || vertexCount == 0)
        {
            return false;
        }

        if (vertexCount > SIZE_MAX / sizeof(Vertex))
        {
            return false;
        }

        if (vertexCount % 3 != 0)
        {
            return false;
        }

        const size_t requiredSize = static_cast<size_t>(vertexCount) * sizeof(Vertex);

        if (m_vertexBuffer->Size() < requiredSize)
        {
            return false;
        }

        struct mygpu_cmd_draw_triangles command;

        command.opcode = MYGPU_CMD_DRAW_TRIANGLES;
        command.vertex_address = m_vertexBuffer->Address();
        command.vertex_count = vertexCount;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

    bool CommandBuffer::BindIndexBuffer(const Buffer &indexBuffer)
    {
        if (m_commandBuffer == nullptr || !indexBuffer.Valid())
        {
            return false;
        }

        m_indexBuffer = &indexBuffer;

        return true;
    }

    bool CommandBuffer::DrawIndexed(uint32_t indexCount)
    {
        if (m_commandBuffer == nullptr ||
            m_vertexBuffer == nullptr ||
            m_indexBuffer == nullptr ||
            indexCount == 0)
        {
            return false;
        }

        if (indexCount % 3 != 0)
        {
            return false;
        }

        if (indexCount > SIZE_MAX / sizeof(uint32_t))
        {
            return false;
        }

        const size_t requiredSize = static_cast<size_t>(indexCount) * sizeof(uint32_t);

        if (m_indexBuffer->Size() < requiredSize)
        {
            return false;
        }

        struct mygpu_cmd_draw_indexed command;

        command.opcode = MYGPU_CMD_DRAW_INDEXED;
        command.vertex_address = m_vertexBuffer->Address();
        command.index_address = m_indexBuffer->Address();
        command.index_count = indexCount;

        return mygpu_command_buffer_write(m_commandBuffer, &command, sizeof(command)) == 0;
    }

} // namespace myrenderer
