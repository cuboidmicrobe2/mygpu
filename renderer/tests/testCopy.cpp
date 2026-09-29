#include <cstdint>

#include "myrenderer/buffer.hpp"
#include "myrenderer/commandBuffer.hpp"
#include "myrenderer/renderer.hpp"
#include "myrenderer/vertex.hpp"

#include "test.hpp"

static void TestCopyBuffer() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 3);

    REQUIRE(source != nullptr);
    REQUIRE(destination != nullptr);

    const uint32_t sourceValues[3] = {0x11111111u, 0x22222222u, 0x33333333u};

    uint32_t destinationValues[3] = {};

    REQUIRE(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->CopyBuffer(*source, 0, *destination, 0, sizeof(sourceValues)));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(destination->Read(0, destinationValues, sizeof(destinationValues)));

    REQUIRE(destinationValues[0] == sourceValues[0]);
    REQUIRE(destinationValues[1] == sourceValues[1]);
    REQUIRE(destinationValues[2] == sourceValues[2]);
}

static void TestCopyBufferWithOffsets() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 4);
    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 4);

    REQUIRE(source != nullptr);
    REQUIRE(destination != nullptr);

    const uint32_t sourceValues[4] = {0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u};

    uint32_t destinationValues[4] = {};

    REQUIRE(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->CopyBuffer(*source, sizeof(uint32_t), *destination, sizeof(uint32_t), sizeof(uint32_t) * 2));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(destination->Read(0, destinationValues, sizeof(destinationValues)));

    REQUIRE(destinationValues[0] == 0);
    REQUIRE(destinationValues[1] == sourceValues[1]);
    REQUIRE(destinationValues[2] == sourceValues[2]);
    REQUIRE(destinationValues[3] == 0);
}

static void TestCopyBufferValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t));
    auto destination = renderer.CreateBuffer(sizeof(uint32_t));

    REQUIRE(source != nullptr);
    REQUIRE(destination != nullptr);

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(!commands->CopyBuffer(*source, 0, *destination, 0, 0));
    REQUIRE(!commands->CopyBuffer(*source, sizeof(uint32_t), *destination, 0, sizeof(uint32_t)));
    REQUIRE(!commands->CopyBuffer(*source, 0, *destination, sizeof(uint32_t), sizeof(uint32_t)));
    REQUIRE(!commands->CopyBuffer(*source, 0, *destination, 0, sizeof(uint32_t) * 2));
}

static void TestCopyBufferDifferentSizes() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto source = renderer.CreateBuffer(sizeof(uint32_t) * 4);
    auto destination = renderer.CreateBuffer(sizeof(uint32_t) * 2);

    REQUIRE(source != nullptr);
    REQUIRE(destination != nullptr);

    const uint32_t sourceValues[4] = {0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u};

    uint32_t destinationValues[2] = {};

    REQUIRE(source->Write(0, sourceValues, sizeof(sourceValues)));

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->CopyBuffer(*source, sizeof(uint32_t), *destination, 0, sizeof(uint32_t) * 2));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    REQUIRE(destination->Read(0, destinationValues, sizeof(destinationValues)));

    REQUIRE(destinationValues[0] == sourceValues[1]);
    REQUIRE(destinationValues[1] == sourceValues[2]);
}

static void TestCopyValidation() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(!commands->Copy(0, 0, 0, 0, 0, 10));
    REQUIRE(!commands->Copy(0, 0, 0, 0, 10, 0));
}

static void TestCopy() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Clear(0x00000000u));
    REQUIRE(commands->DrawRect(1, 1, 2, 2, 0xFF0000FFu));
    REQUIRE(commands->Copy(1, 1, 5, 5, 2, 2));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(5, 5, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(6, 5, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(5, 6, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(6, 6, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(0, 0, color));
    REQUIRE(color == 0x00000000u);
}

static void TestCopyOverlap() {
    myrenderer::Renderer renderer;

    REQUIRE(renderer.Valid());

    auto commands = renderer.CreateCommandBuffer(256);

    REQUIRE(commands != nullptr);
    REQUIRE(commands->Valid());

    REQUIRE(commands->Clear(0x00000000u));
    REQUIRE(commands->DrawRect(1, 1, 3, 1, 0xFF0000FFu));
    REQUIRE(commands->Copy(1, 1, 2, 1, 3, 1));

    REQUIRE(renderer.BeginFrame());
    REQUIRE(renderer.EndFrame(*commands));

    uint32_t color = 0;

    REQUIRE(renderer.GetPixel(1, 1, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(2, 1, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(3, 1, color));
    REQUIRE(color == 0xFF0000FFu);

    REQUIRE(renderer.GetPixel(4, 1, color));
    REQUIRE(color == 0xFF0000FFu);
}

int main() {
    RUN_TEST(TestCopyBuffer);
    RUN_TEST(TestCopyBufferWithOffsets);
    RUN_TEST(TestCopyBufferValidation);
    RUN_TEST(TestCopyBufferDifferentSizes);
    RUN_TEST(TestCopyValidation);
    RUN_TEST(TestCopy);
    RUN_TEST(TestCopyOverlap);

    return TEST_FINISH();
}
