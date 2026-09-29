CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude

BUILD_DIR = build
TEST_BUILD_DIR = $(BUILD_DIR)/tests
LIBRARY = $(BUILD_DIR)/libmygpu.a

# --------------------------------------------------
# GPU sources
# --------------------------------------------------

GPU_CORE_SOURCES = \
	gpu/memory.c \
	gpu/registers.c \
	gpu/framebuffer.c

GPU_SOURCES = \
	$(GPU_CORE_SOURCES) \
	gpu/gpu.c \
	gpu/fence.c \
	gpu/queue.c \
	gpu/commands.c \
	gpu/buffer.c \
	gpu/vertex.c \
	gpu/rasterizer.c

GPU_OBJECTS = \
	$(GPU_SOURCES:gpu/%.c=$(BUILD_DIR)/gpu/%.o)

# --------------------------------------------------
# Library
# --------------------------------------------------

$(BUILD_DIR)/gpu/%.o: gpu/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(GPU_OBJECTS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

library: $(LIBRARY)

# --------------------------------------------------
# Test executables
# --------------------------------------------------

GPU_TEST = $(TEST_BUILD_DIR)/test_gpu
MEMORY_TEST = $(TEST_BUILD_DIR)/test_memory
REGISTERS_TEST = $(TEST_BUILD_DIR)/test_registers
FRAMEBUFFER_TEST = $(TEST_BUILD_DIR)/test_framebuffer
BUFFER_TEST = $(TEST_BUILD_DIR)/test_buffer
VERTEX_BUFFER_TEST = $(TEST_BUILD_DIR)/test_vertex_buffer
COMMANDS_TEST = $(TEST_BUILD_DIR)/test_commands
QUEUE_TEST = $(TEST_BUILD_DIR)/test_queue
DRAW_TRIANGLES_TEST = $(TEST_BUILD_DIR)/test_draw_triangles
VERTEX_TEST = $(TEST_BUILD_DIR)/test_vertex
RASTERIZER_TEST = $(TEST_BUILD_DIR)/test_rasterizer
FENCE_TEST = $(TEST_BUILD_DIR)/test_fence
SCISSOR_TEST = $(TEST_BUILD_DIR)/test_scissor

# --------------------------------------------------
# Test source files
# --------------------------------------------------

GPU_TEST_SOURCE = tests/test_gpu.c
MEMORY_TEST_SOURCE = tests/test_memory.c
REGISTERS_TEST_SOURCE = tests/test_registers.c
FRAMEBUFFER_TEST_SOURCE = tests/test_framebuffer.c
BUFFER_TEST_SOURCE = tests/test_buffer.c
VERTEX_BUFFER_TEST_SOURCE = tests/test_vertex_buffer.c
COMMANDS_TEST_SOURCE = tests/test_commands.c
QUEUE_TEST_SOURCE = tests/test_queue.c
DRAW_TRIANGLES_TEST_SOURCE = tests/test_draw_triangles.c
VERTEX_TEST_SOURCE = tests/test_vertex.c
RASTERIZER_TEST_SOURCE = tests/test_rasterizer.c
FENCE_TEST_SOURCE = tests/test_fence.c
SCISSOR_TEST_SOURCE = tests/test_scissor.c

# --------------------------------------------------
# Phony targets
# --------------------------------------------------

.PHONY: all library test \
	test-gpu \
	test-memory \
	test-registers \
	test-framebuffer \
	test-buffer \
	test-vertex-buffer \
	test-commands \
	test-queue \
	test-draw-triangles \
	test-vertex \
	test-rasterizer \
	test-fence \
	test-scissor \
	clean

# --------------------------------------------------
# Default
# --------------------------------------------------

all: $(LIBRARY) \
	$(GPU_TEST) \
	$(MEMORY_TEST) \
	$(REGISTERS_TEST) \
	$(FRAMEBUFFER_TEST) \
	$(BUFFER_TEST) \
	$(VERTEX_BUFFER_TEST) \
	$(COMMANDS_TEST) \
	$(QUEUE_TEST) \
	$(DRAW_TRIANGLES_TEST) \
	$(VERTEX_TEST) \
	$(RASTERIZER_TEST) \
	$(FENCE_TEST) \
	$(SCISSOR_TEST)

# --------------------------------------------------
# Build tests
# --------------------------------------------------

$(GPU_TEST): $(LIBRARY) $(GPU_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GPU_TEST_SOURCE) $(LIBRARY) -o $@

$(MEMORY_TEST): gpu/memory.c $(MEMORY_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/memory.c $(MEMORY_TEST_SOURCE) -o $@

$(REGISTERS_TEST): gpu/registers.c $(REGISTERS_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/registers.c $(REGISTERS_TEST_SOURCE) -o $@

$(FRAMEBUFFER_TEST): gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) gpu/framebuffer.c $(FRAMEBUFFER_TEST_SOURCE) -o $@

$(BUFFER_TEST): $(LIBRARY) $(BUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(BUFFER_TEST_SOURCE) $(LIBRARY) -o $@

$(VERTEX_BUFFER_TEST): $(LIBRARY) $(VERTEX_BUFFER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(VERTEX_BUFFER_TEST_SOURCE) $(LIBRARY) -o $@

$(COMMANDS_TEST): $(LIBRARY) $(COMMANDS_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(COMMANDS_TEST_SOURCE) $(LIBRARY) -o $@

$(QUEUE_TEST): $(LIBRARY) $(QUEUE_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(QUEUE_TEST_SOURCE) $(LIBRARY) -o $@

$(DRAW_TRIANGLES_TEST): $(LIBRARY) $(DRAW_TRIANGLES_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DRAW_TRIANGLES_TEST_SOURCE) $(LIBRARY) -o $@

$(VERTEX_TEST): $(LIBRARY) $(VERTEX_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(VERTEX_TEST_SOURCE) $(LIBRARY) -o $@

$(RASTERIZER_TEST): $(LIBRARY) $(RASTERIZER_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(RASTERIZER_TEST_SOURCE) $(LIBRARY) -o $@

$(FENCE_TEST): $(LIBRARY) $(FENCE_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(FENCE_TEST_SOURCE) $(LIBRARY) -o $@

$(SCISSOR_TEST): $(LIBRARY) $(SCISSOR_TEST_SOURCE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SCISSOR_TEST_SOURCE) $(LIBRARY) -o $@

# --------------------------------------------------
# Run tests
# --------------------------------------------------

TEST_BINARIES = \
	$(GPU_TEST) \
	$(MEMORY_TEST) \
	$(REGISTERS_TEST) \
	$(FRAMEBUFFER_TEST) \
	$(BUFFER_TEST) \
	$(VERTEX_BUFFER_TEST) \
	$(COMMANDS_TEST) \
	$(QUEUE_TEST) \
	$(DRAW_TRIANGLES_TEST) \
	$(VERTEX_TEST) \
	$(RASTERIZER_TEST) \
	$(FENCE_TEST) \
	$(SCISSOR_TEST)

$(TEST_BINARIES): tests/test.h

# Run every suite even if one fails, then report the overall result.
test: $(TEST_BINARIES)
	@failed=0; \
	for t in $(TEST_BINARIES); do ./$$t || failed=1; done; \
	if [ $$failed -eq 0 ]; then echo "all tests passed"; else echo "some tests failed"; exit 1; fi

test-gpu: $(GPU_TEST)
	@./$(GPU_TEST)

test-memory: $(MEMORY_TEST)
	@./$(MEMORY_TEST)

test-registers: $(REGISTERS_TEST)
	@./$(REGISTERS_TEST)

test-framebuffer: $(FRAMEBUFFER_TEST)
	@./$(FRAMEBUFFER_TEST)

test-buffer: $(BUFFER_TEST)
	@./$(BUFFER_TEST)

test-vertex-buffer: $(VERTEX_BUFFER_TEST)
	@./$(VERTEX_BUFFER_TEST)

test-commands: $(COMMANDS_TEST)
	@./$(COMMANDS_TEST)

test-queue: $(QUEUE_TEST)
	@./$(QUEUE_TEST)

test-draw-triangles: $(DRAW_TRIANGLES_TEST)
	@./$(DRAW_TRIANGLES_TEST)

test-vertex: $(VERTEX_TEST)
	@./$(VERTEX_TEST)

test-rasterizer: $(RASTERIZER_TEST)
	@./$(RASTERIZER_TEST)

test-fence: $(FENCE_TEST)
	@./$(FENCE_TEST)

test-scissor: $(SCISSOR_TEST)
	@./$(SCISSOR_TEST)

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)