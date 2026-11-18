CXX    := g++
CC     := gcc
TARGET := EnGen7Editor.exe
BUILD  := build

RAYLIB_DIR := ../thirdparty/raylib-desktop/src
ENGINE     := ../Engine

ENGINE_SRC_DIRS := $(ENGINE)/src $(ENGINE)/src/ECS $(ENGINE)/src/Systems
EDITOR_SRC_DIRS := src
INC_DIRS := src $(ENGINE)/include $(ENGINE)/include/ECS $(ENGINE)/include/Systems $(RAYLIB_DIR)

ENGINE_CPP := $(filter-out main.cpp,$(notdir $(foreach d,$(ENGINE_SRC_DIRS),$(wildcard $(d)/*.cpp))))
ENGINE_C   := $(notdir $(foreach d,$(ENGINE_SRC_DIRS),$(wildcard $(d)/*.c)))
EDITOR_CPP := $(notdir $(foreach d,$(EDITOR_SRC_DIRS),$(wildcard $(d)/*.cpp)))

ENGINE_OBJS := $(addprefix $(BUILD)/,$(ENGINE_CPP:.cpp=.o) $(ENGINE_C:.c=.o))
EDITOR_OBJS := $(addprefix $(BUILD)/,$(EDITOR_CPP:.cpp=.o))

# editor dirs first, so its main.cpp wins over the engine's
vpath %.cpp $(EDITOR_SRC_DIRS) $(ENGINE_SRC_DIRS)
vpath %.c   $(ENGINE_SRC_DIRS)

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) -DENGEN7_DESKTOP -MMD -MP
CFLAGS   := -g -O0 -Wall
CXXFLAGS := $(CFLAGS) -std=c++20 -fno-rtti -fno-exceptions
LDLIBS   := -static -L$(RAYLIB_DIR) -lraylib -lopengl32 -lgdi32 -lwinmm

$(TARGET): $(EDITOR_OBJS) $(ENGINE_OBJS)
	$(CXX) $^ -o $@ $(LDLIBS)

engine: $(ENGINE_OBJS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: engine clean
-include $(wildcard $(BUILD)/*.d)