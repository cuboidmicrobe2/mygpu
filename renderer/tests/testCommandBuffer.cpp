#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

#include "test.hpp"

static void TestCommandBufferClearAndReset() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(commandBuffer != nullptr);
    REQUIRE(commandBuffer->Valid());
    REQUIRE(commandBuffer->IsEmpty());

    constexpr uint32_t commandClearColor = 0x12345678u;

    REQUIRE(commandBuffer->Clear(commandClearColor));
    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == commandClearColor);

    REQUIRE(commandBuffer->Reset());
    REQUIRE(commandBuffer->IsEmpty());

    REQUIRE(commandBuffer->Clear(0x11223344u));
    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x11223344u);
}

static void TestMultipleCommands() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commandBuffer = renderer.CreateCommandBuffer(1024);

    REQUIRE(commandBuffer != nullptr);

    constexpr uint32_t firstRectColor = 0x11112222u;
    constexpr uint32_t secondRectColor = 0x33334444u;

    REQUIRE(commandBuffer->Clear(0x00000000u));

    REQUIRE(commandBuffer->DrawRect(5, 5, 10, 10, firstRectColor));

    REQUIRE(commandBuffer->DrawRect(30, 5, 10, 10, secondRectColor));

    REQUIRE(!commandBuffer->IsEmpty());

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commandBuffer));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(5, 5, color));
    REQUIRE(color == firstRectColor);

    REQUIRE(renderer.GetPixel(30, 5, color));
    REQUIRE(color == secondRectColor);

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x00000000u);
}

static void TestCommandBufferResetState() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);

    REQUIRE(vertexBuffer != nullptr);

    auto commandBuffer = renderer.CreateCommandBuffer(256);

    REQUIRE(commandBuffer != nullptr);

    REQUIRE(commandBuffer->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commandBuffer->DrawTriangles(3, 0));

    REQUIRE(commandBuffer->Reset());
    REQUIRE(commandBuffer->IsEmpty());

    REQUIRE(!commandBuffer->DrawTriangles(3, 0));
}

static void TestCommandBufferReuse() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());
    REQUIRE(commands->Clear(0x12345678u));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));
}

static void TestCommandBufferCapacity() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(sizeof(uint32_t));

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Present());
    REQUIRE(!commands->Present());
}

static void TestCommandBufferResetCapacity() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(sizeof(uint32_t));

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Present());
    REQUIRE(!commands->Present());

    REQUIRE(commands->Reset());
    REQUIRE(commands->IsEmpty());

    REQUIRE(commands->Present());
}

static void TestResetClearsAllBindings() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    const uint32_t indices[3] = {0, 1, 2};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);
    auto indexBuffer = renderer.CreateIndexBuffer(indices, 3);

    REQUIRE(vertexBuffer != nullptr);
    REQUIRE(indexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->BindIndexBuffer(*indexBuffer));
    REQUIRE(commands->DrawIndexed(3, 0));

    REQUIRE(commands->Reset());

    REQUIRE(!commands->DrawTriangles(3, 0));
    REQUIRE(!commands->DrawIndexed(3, 0));
}

static void TestCommandBufferReuseAfterReset() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto vertexBuffer = renderer.CreateVertexBuffer(vertices, 3);

    REQUIRE(vertexBuffer != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->DrawTriangles(3, 0));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(commands->Reset());
    REQUIRE(commands->BindVertexBuffer(*vertexBuffer, 0));
    REQUIRE(commands->DrawTriangles(3, 0));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));
}

int main() {
    RUN_TEST(TestCommandBufferClearAndReset);
    RUN_TEST(TestMultipleCommands);
    RUN_TEST(TestCommandBufferResetState);
    RUN_TEST(TestCommandBufferReuse);
    RUN_TEST(TestCommandBufferCapacity);
    RUN_TEST(TestCommandBufferResetCapacity);
    RUN_TEST(TestResetClearsAllBindings);
    RUN_TEST(TestCommandBufferReuseAfterReset);

    return TEST_FINISH();
}
