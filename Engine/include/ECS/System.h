#pragma once
#include "Types.h"
#include <set>

class System {
public:
    virtual ~System() = default;
    virtual void update(float dt) = 0;

    virtual void onEntityAdded(Entity e) {
        mEntities.insert(e);
    }
    virtual void onEntityRemoved(Entity e) {
        mEntities.erase(e);
    }

    std::set<Entity> mEntities;
};