#include "ComponentManager.h"

void ComponentManager::entityDestroyed(Entity e) {
    for (size_t i = 0; i < MAX_COMPONENTS; ++i)
        if (mArrays[i]) mArrays[i]->entityDestroyed(e);
}