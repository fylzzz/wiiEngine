CXX := g++

RAYLIB_DIR := ../thirdparty/raylib-desktop/src

CXXFLAGS := -std=c++20 -g -O0 -Wall -I$(RAYLIB_DIR)
LIBS := -static -L$(RAYLIB_DIR) -lraylib -lopengl32 -lgdi32 -lwinmm


test.exe: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)