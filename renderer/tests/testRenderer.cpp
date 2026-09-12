#include <cassert>
#include <cstdint>
#include <memory>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"

int main()
{
    myrenderer::Renderer renderer;

    assert(renderer.Valid());

    assert(renderer.Width() > 0);
    assert(renderer.Height() > 0);

    constexpr uint32_t clearColor = 0xAABBCCDDu;

    assert(renderer.Clear(clearColor));

    uint32_t color = 0;

    assert(renderer.GetPixel(0, 0, color));
    assert(color == clearColor);

    auto buffer = renderer.CreateBuffer(3 * sizeof(uint32_t));

    assert(buffer != nullptr);
    assert(buffer->Valid());
    assert(buffer->Size() == 3 * sizeof(uint32_t));

    const uint32_t values[3] = {
        0x11111111u,
        0x22222222u,
        0x33333333u};

    assert(buffer->Write(0, values, sizeof(values)));

    uint32_t result[3] = {};

    assert(buffer->Read(0, result, sizeof(result)));

    assert(result[0] == values[0]);
    assert(result[1] == values[1]);
    assert(result[2] == values[2]);

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    assert(commandBuffer != nullptr);
    assert(commandBuffer->Valid());

    constexpr uint32_t commandClearColor = 0x12345678u;

    assert(commandBuffer->Clear(commandClearColor));

    assert(renderer.Submit(*commandBuffer));

    assert(renderer.GetPixel(0, 0, color));
    assert(color == commandClearColor);

    return 0;
}