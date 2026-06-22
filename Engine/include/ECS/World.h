#pragma once

#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
#include "ResourceManager.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "Serializer.h"
#include "Types.h"

#include <memory>


struct ComponentSerializer {
	void  (*serialize)  (Writer& w, void* arrayPtr, Entity e) = nullptr;
	void  (*deserialize)(Reader& r, void* arrayPtr, Entity e) = nullptr;
	bool  (*has)        (void* arrayPtr, Entity e) = nullptr;
	void* arrayPtr = nullptr;
};

class World {
	public:
		void init();

		// Entity API calls
		Entity createEntity();
		void destroyEntity(Entity e);

		// Component API calls
		template<typename T> void registerComponent();
		template<typename T> void addComponent(Entity e, T component);
		template<typename T> void removeComponent(Entity e);
		template<typename T> T& getComponent(Entity e);
		template<typename T> ComponentType getComponentType() const;
		template<typename T> bool hasComponent(Entity e);

		// System API calls
		template<typename T> std::shared_ptr<T> registerSystem();
		template<typename T> void setSystemSignature(Signature sig);

		// Resouce API calls
		ResourceId loadModel(const char* path);
		Model& getModel(ResourceId id);
		void unloadAllResources();

		// Sprite API calls
		SpriteId loadSprite(const char* path);
		Image& getSprite(SpriteId id);
		void unloadAllSprites();

		// Animation API calls
		AnimId loadAnim(std::string name, const char* path, int frameCount, int fps);
		AnimationClip& getAnim(AnimId id);
		void unloadAllAnims();

		// Serializer API calls
		template<typename T> void registerComponentSerializer();
		void save(const char* path);
		void load(const char* path);

	private:
		std::unique_ptr<EntityManager> mEntityManager;
		std::unique_ptr<ComponentManager> mComponentManager;
		std::unique_ptr<SystemManager> mSystemManager;
		std::unique_ptr<ResourceManager> mResourceManager;
		std::unique_ptr<SpriteManager> mSpriteManager;
		std::unique_ptr<AnimationManager> mAnimationManager;

		std::array<ComponentSerializer, MAX_COMPONENTS> mSerializers{};
};


template<typename T>
void World::registerComponent() {
	mComponentManager->registerComponent<T>();
}

template<typename T>
void World::addComponent(Entity e, T component) {
	mComponentManager->addComponent<T>(e, component);

	Signature sig = mEntityManager->getSignature(e);
	sig.set(mComponentManager->getComponentType<T>());
	mEntityManager->setSignature(e, sig);
	mSystemManager->entitySignatureChanged(e, sig);
}

template<typename T>
void World::removeComponent(Entity e) {
	mComponentManager->removeComponent<T>(e);

	Signature sig = mEntityManager->getSignature(e);
	sig.reset(mComponentManager->getComponentType<T>());
	mEntityManager->setSignature(e, sig);
	mSystemManager->entitySignatureChanged(e, sig);
}

template<typename T>
T& World::getComponent(Entity e) {
	return mComponentManager->getComponent<T>(e);
}

template<typename T>
ComponentType World::getComponentType() const {
	return mComponentManager->getComponentType<T>();
}

template<typename T>
bool World::hasComponent(Entity e) {
	ComponentType id = mComponentManager->getComponentType<T>();
	if (mSerializers[id].has == nullptr) return false;
	return mSerializers[id].has(mSerializers[id].arrayPtr, e);
}

template<typename T>
std::shared_ptr<T> World::registerSystem() {
	return mSystemManager->registerSystem<T>();
}

template<typename T>
void World::setSystemSignature(Signature sig) {
	mSystemManager->setSignature<T>(sig);
}

template<typename T>
void World::registerComponentSerializer() {
	ComponentType id = mComponentManager->getComponentType<T>();

	mSerializers[id].arrayPtr = mComponentManager->getRawArray<T>();

	mSerializers[id].has = [](void* ptr, Entity e) -> bool {
		return static_cast<ComponentArray<T>*>(ptr)->hasComponent(e);
		};

	mSerializers[id].serialize = [](Writer& w, void* ptr, Entity e) {
		T& comp = static_cast<ComponentArray<T>*>(ptr)->getComponent(e);
		w.write(comp);
		};

	mSerializers[id].deserialize = [](Reader& r, void* ptr, Entity e) {
		alignas(T) unsigned char buf[sizeof(T)];
		r.read(*reinterpret_cast<T*>(buf));
		static_cast<ComponentArray<T>*>(ptr)->insertComponent(e, *reinterpret_cast<T*>(buf));
		};
}