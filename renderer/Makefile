CXX = g++

CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -g \
	-Iinclude \
	-I../include

LIBRARY = ../build/libmygpu.a

BUILD_DIR = ../build/renderer
TARGET = $(BUILD_DIR)/test_renderer

SOURCES = \
	src/renderer.cpp \
	tests/test_renderer.cpp

OBJECTS = \
	$(BUILD_DIR)/renderer.o \
	$(BUILD_DIR)/test_renderer.o

.PHONY: all test clean

all: $(TARGET)

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) $(LIBRARY)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBRARY) -o $@

$(BUILD_DIR)/renderer.o: src/renderer.cpp include/myrenderer/renderer.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/test_renderer.o: tests/test_renderer.cpp include/myrenderer/renderer.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)