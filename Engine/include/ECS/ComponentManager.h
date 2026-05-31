#pragma once

#include "ComponentArray.h"
#include "Types.h"
#include "TypeId.h"

#include <array>
#include <memory>


class ComponentManager {
public:
    template<typename T>
    void registerComponent() {
        ComponentType id = TypeIdGenerator::get<T>();
        assert(!mRegistered[id] && "Component registered twice.");
        mRegistered[id] = true;
        mArrays[id] = std::make_shared<ComponentArray<T>>();
    }

    template<typename T>
    ComponentType getComponentType() const {
        return TypeIdGenerator::get<T>();
    }

    template<typename T>
    void addComponent(Entity e, T component) {
        getArray<T>()->insertComponent(e, component);
    }

    template<typename T>
    void removeComponent(Entity e) {
        getArray<T>()->removeComponent(e);
    }

    template<typename T>
    T& getComponent(Entity e) {
        return getArray<T>()->getComponent(e);
    }

    template<typename T>
    ComponentArray<T>* getRawArray() {
        ComponentType id = TypeIdGenerator::get<T>();
        return static_cast<ComponentArray<T>*>(mArrays[id].get());
    }

    void entityDestroyed(Entity e);

private:
    // Fixed-size arrays indexed directly by ComponentType — no map needed.
    std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS> mArrays{};
    std::array<bool, MAX_COMPONENTS>                             mRegistered{};

    template<typename T>
    std::shared_ptr<ComponentArray<T>> getArray() const {
        ComponentType id = TypeIdGenerator::get<T>();
        assert(mRegistered[id] && "Component not registered.");
        return std::static_pointer_cast<ComponentArray<T>>(mArrays[id]);
    }

};