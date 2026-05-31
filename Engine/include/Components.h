#pragma once

#include <raylib.h>
#include "ResourceManager.h"

struct EngineTransform {
	Vector3 pos;
	Vector3 rot;
	Vector3 sca;
};

// --------------------
// Render Components
// --------------------

enum class RenderShape {Cube, Sphere, Cylinder, Capsule, Plane, Model, ModelWires};

struct Renderable {
    RenderShape shape = RenderShape::Cube;
    Color color = WHITE;
    union {
        struct { float width, height, length; } cube;
        struct { float radius; } sphere;
        struct { float topRadius, bottomRadius, height; int slices; } cylinder;
        struct { float height; float radius; int slices, rings; } capsule;
        struct { Vector2 size; } plane;
        struct { ResourceId modelId; float scale; } model;
    };

    Renderable() : shape(RenderShape::Cube), color(WHITE) {
        cube.width = cube.height = cube.length = 1.0f;
    }
};