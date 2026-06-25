#pragma once

#include <map>

#include <raylib.h>
#include "ResourceManager.h"
#include "SpriteManager.h"

struct EngineTransform {
	Vector3 pos;
	Vector3 rot;
	Vector3 sca;
};

// --------------------
// Render Components
// --------------------

enum class RenderShape2D {Rectangle, Circle, Triangle, Ellipse};

struct PrimitiveRenderable2D {
    RenderShape2D shape = RenderShape2D::Rectangle;
    Color color = WHITE;
    union {
        struct { int width, height; } rectangle;
        struct { float radius; } circle;
        struct { Vector2 v1, v2, v3; } triangle;
        struct { float radiusH, radiusV; } ellipse;
    };
};

struct Renderable2D {
    SpriteId spriteId;
    Color color;
    Texture2D texture;
};

struct Animator2D {
    std::string currentAnim;
    std::map<std::string, AnimId> animClips;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    bool finished = false;
    Color color = WHITE;
};

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

// ---------------------
// Physics Components
// ---------------------

enum RbType { Dynamic, Kinematic, Static };

struct RigidBody2D {
    RbType type;
    float mass;
    Vector2 velocity;
};

struct Collider2D {
    Entity entityId;
    Rectangle bounds;
    Vector2 offset;
};