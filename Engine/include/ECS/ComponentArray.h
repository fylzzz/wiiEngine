#include "Types.h"

#include <unordered_map>
#include <array>
#include <cassert>


class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void entityDestroyed(Entity e) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
	public:
		void insertComponent(Entity e, T component);
		void removeComponent(Entity e);
		T& getComponent(Entity e);

		void entityDestroyed(Entity e) override;

		bool hasComponent(Entity e) const {
			return mEntitytoIndexMap.find(e) != mEntitytoIndexMap.end();
		}

	private:
		std::array<T, MAX_ENTITIES> mComponentArray;
		std::unordered_map<Entity, size_t> mEntitytoIndexMap;
		std::unordered_map<size_t, Entity> mIndextoEntityMap;
		size_t mSize = 0;
};


template<typename T>
void ComponentArray<T>::insertComponent(Entity e, T component) {
	assert(mEntitytoIndexMap.find(e) == mEntitytoIndexMap.end() && "Duplicate Component added to Entity");
	size_t newIndex = mSize;
	mEntitytoIndexMap[e] = newIndex;
	mIndextoEntityMap[newIndex] = e;
	mComponentArray[newIndex] = component;
	++mSize;
}

template<typename T>
void ComponentArray<T>::removeComponent(Entity e) {
	assert(mEntitytoIndexMap.find(e) != mEntitytoIndexMap.end() && "Component does not exist in Entity");
	// swap component to remove with final in array to prevent holes
	size_t indexToRemove = mEntitytoIndexMap[e];
	size_t indexOfLast = mSize - 1;
	mComponentArray[indexToRemove] = mComponentArray[indexOfLast];

	// update map for moved component
	Entity entityOfLast = mIndextoEntityMap[indexOfLast];
	mEntitytoIndexMap[entityOfLast] = indexToRemove;
	mIndextoEntityMap[indexToRemove] = entityOfLast;

	// remove component
	mEntitytoIndexMap.erase(e);
	mIndextoEntityMap.erase(indexOfLast);
	--mSize;
}

template<typename T>
T& ComponentArray<T>::getComponent(Entity e) {
	assert(mEntitytoIndexMap.find(e) != mEntitytoIndexMap.end() && "Component does not exist in Entity");
	return mComponentArray[mEntitytoIndexMap[e]];
}

template<typename T>
void ComponentArray<T>::entityDestroyed(Entity e) {
	if (mEntitytoIndexMap.find(e) != mEntitytoIndexMap.end()) {
		removeComponent(e);
	}
}