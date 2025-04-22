# Compilers
CXX = g++
CC = gcc
CXXFLAGS = -Wall -std=c++17 -g
CFLAGS = -Wall -std=c11 -g

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
IMGUI_DIR = include/imgui
IMGUI_BACKENDS_DIR = $(IMGUI_DIR)/backends

# Include paths
INCLUDE_DIRS = -Iinclude -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS_DIR) -I/usr/include -I/usr/local/include

# Libraries
LIB_DIRS = -L/usr/lib
LIBS = -lglfw -lGL -lGLEW -lglut -lX11 -ldl -lm -pthread

# Source files
SRCS_CPP = $(wildcard $(SRC_DIR)/*.cpp)
SRCS_C = $(wildcard $(SRC_DIR)/*.c)  # For glad.c

# ImGui source files
IMGUI_SRCS = \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_BACKENDS_DIR)/imgui_impl_opengl3.cpp \
	$(IMGUI_BACKENDS_DIR)/imgui_impl_glut.cpp

# Object files
OBJS_CPP = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_CPP))
OBJS_C = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS_C))
OBJS_IMGUI = $(patsubst include/%.cpp, $(BUILD_DIR)/%.o, $(IMGUI_SRCS))

# Final target
TARGET = $(BIN_DIR)/main

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS_CPP) $(OBJS_C) $(OBJS_IMGUI) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS_CPP) $(OBJS_C) $(OBJS_IMGUI) -o $@ $(LIB_DIRS) $(LIBS)

# Compile project .cpp files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile project .c files (e.g., glad.c)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile ImGui .cpp files
$(BUILD_DIR)/imgui/%.o: include/imgui/%.cpp | $(BUILD_DIR)/imgui
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/imgui/backends/%.o: include/imgui/backends/%.cpp | $(BUILD_DIR)/imgui/backends
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Create necessary directories
$(BIN_DIR) $(BUILD_DIR) $(BUILD_DIR)/imgui $(BUILD_DIR)/imgui/backends:
	mkdir -p $@

# Utility targets
run: all
	./$(TARGET)

debug: all
	gdb ./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean run debug