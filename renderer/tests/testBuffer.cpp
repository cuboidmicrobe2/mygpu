#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

#include "test.hpp"

static void TestBuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto buffer = renderer.CreateBuffer(3 * sizeof(uint32_t));

    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->Valid());
    REQUIRE(buffer->Size() == 3 * sizeof(uint32_t));

    const uint32_t values[3] = {0x11111111u, 0x22222222u, 0x33333333u};

    REQUIRE(buffer->Write(0, values, sizeof(values)));

    uint32_t result[3] = {};

    REQUIRE(buffer->Read(0, result, sizeof(result)));

    REQUIRE(result[0] == values[0]);
    REQUIRE(result[1] == values[1]);
    REQUIRE(result[2] == values[2]);
}

static void TestBufferLifetime() {
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
}

static void TestWriteIndices() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto buffer = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->Valid());

    const uint32_t indices[3] = {0, 1, 2};
    uint32_t readBack[3] = {};

    REQUIRE(buffer->WriteIndices(indices, 3));
    REQUIRE(buffer->Read(0, readBack, sizeof(readBack)));

    REQUIRE(readBack[0] == 0);
    REQUIRE(readBack[1] == 1);
    REQUIRE(readBack[2] == 2);
}

static void TestWriteIndicesValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto buffer = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->Valid());

    const uint32_t indices[3] = {0, 1, 2};

    REQUIRE(!buffer->WriteIndices(nullptr, 3));
    REQUIRE(!buffer->WriteIndices(indices, 0));
}

static void TestCreateIndexBuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const uint32_t indices[3] = {0, 1, 2};

    auto buffer = renderer.CreateIndexBuffer(indices, 3);

    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->Valid());
    REQUIRE(buffer->Size() == sizeof(indices));

    uint32_t readBack[3] = {};

    REQUIRE(buffer->Read(0, readBack, sizeof(readBack)));

    REQUIRE(readBack[0] == 0);
    REQUIRE(readBack[1] == 1);
    REQUIRE(readBack[2] == 2);
}

static void TestCreateIndexBufferValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const uint32_t indices[3] = {0, 1, 2};

    REQUIRE(renderer.CreateIndexBuffer(nullptr, 3) == nullptr);
    REQUIRE(renderer.CreateIndexBuffer(indices, 0) == nullptr);
}

static void TestCreateVertexBuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    auto buffer = renderer.CreateVertexBuffer(vertices, 3);

    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->Valid());
    REQUIRE(buffer->Size() == sizeof(vertices));

    myrenderer::Vertex readBack[3] = {};

    REQUIRE(buffer->Read(0, readBack, sizeof(readBack)));

    REQUIRE(readBack[0].x == 10.0f);
    REQUIRE(readBack[0].y == 10.0f);
    REQUIRE(readBack[1].x == 30.0f);
    REQUIRE(readBack[2].y == 30.0f);
}

static void TestCreateVertexBufferValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    const myrenderer::Vertex vertices[3] = {{10.0f, 10.0f, 0xFFFFFFFFu},
                                            {30.0f, 10.0f, 0xFFFFFFFFu},
                                            {20.0f, 30.0f, 0xFFFFFFFFu}};

    REQUIRE(renderer.CreateVertexBuffer(nullptr, 3) == nullptr);
    REQUIRE(renderer.CreateVertexBuffer(vertices, 0) == nullptr);
}

int main() {
    RUN_TEST(TestBuffer);
    RUN_TEST(TestBufferLifetime);
    RUN_TEST(TestWriteIndices);
    RUN_TEST(TestWriteIndicesValidation);
    RUN_TEST(TestCreateIndexBuffer);
    RUN_TEST(TestCreateIndexBufferValidation);
    RUN_TEST(TestCreateVertexBuffer);
    RUN_TEST(TestCreateVertexBufferValidation);

    return TEST_FINISH();
}
