#pragma once

#include "System.h"
#include "World.h"
#include "Components.h"
#include <raylib.h>

class RenderSystem : public System {
public:
    World* world = nullptr;
    Camera3D* camera = nullptr;
    void update(float dt) override;

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;

private:
    std::multimap<int, Entity> mEntityZMap;
    std::unordered_map<Entity, int> mEntityZIndex;
};