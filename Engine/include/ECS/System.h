#pragma once
#include "Types.h"
#include <set>

class System {
public:
    virtual ~System() = default;
    virtual void update(float dt) = 0;
    std::set<Entity> mEntities;
};