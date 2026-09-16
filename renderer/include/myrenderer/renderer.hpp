#pragma once

#include <cstdint>
#include <memory>

struct mygpu;
struct mygpu_queue;
struct mygpu_fence;

namespace myrenderer
{
    class Buffer;
    class CommandBuffer;
    struct Vertex;

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

        bool Presented() const;

        bool Clear(uint32_t color);

        bool BeginFrame();

        bool EndFrame(const CommandBuffer &commandBuffer);

        bool GetPixel(uint32_t x, uint32_t y, uint32_t &color) const;

        std::unique_ptr<Buffer> CreateBuffer(size_t size);

        std::unique_ptr<Buffer> CreateVertexBuffer(const Vertex *vertices, size_t vertexCount);

        std::unique_ptr<CommandBuffer> CreateCommandBuffer(size_t size);

    private:
        bool Submit(const CommandBuffer &commandBuffer);

        struct mygpu *m_gpu;
        struct mygpu_queue *m_queue;
        struct mygpu_fence *m_fence;
        bool m_frameActive;
    };
} // namespace myrenderer