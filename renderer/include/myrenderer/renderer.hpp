#pragma once

#include <cstdint>
#include <memory>

struct mygpu;
struct mygpu_queue;

namespace myrenderer
{
    class Buffer;
    class CommandBuffer;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        bool Valid() const;

        uint32_t Width() const;
        uint32_t Height() const;

        bool Clear(uint32_t color);

        bool GetPixel(uint32_t x, uint32_t y, uint32_t &color) const;

        std::unique_ptr<Buffer> CreateBuffer(size_t size);
        std::unique_ptr<CommandBuffer> CreateCommandBuffer(size_t size);

        bool Submit(const CommandBuffer &commandBuffer);

    private:
        struct mygpu *m_gpu;
        struct mygpu_queue *m_queue;
    };
} // namespace myrenderer