#include "World.h"


void World::init() {
	mEntityManager = std::make_unique<EntityManager>();
	mComponentManager = std::make_unique<ComponentManager>();
	mSystemManager = std::make_unique<SystemManager>();
	mResourceManager = std::make_unique<ResourceManager>();
    mSpriteManager = std::make_unique<SpriteManager>();
    mAnimationManager = std::make_unique<AnimationManager>();
}

Entity World::createEntity() {
	return mEntityManager->create();
}

void World::destroyEntity(Entity e) {
	mEntityManager->destroy(e);
	mComponentManager->entityDestroyed(e);
	mSystemManager->entityDestroyed(e);
}

ResourceId World::loadModel(const char* path) {
	return mResourceManager->loadModel(path);
}

Model& World::getModel(ResourceId id) {
	return mResourceManager->getModel(id);
}

void World::unloadAllResources() {
	return mResourceManager->unloadAll();
}

SpriteId World::loadSprite(const char* path) {
    return mSpriteManager->loadSprite(path);
}

Image& World::getSprite(SpriteId id) {
    return mSpriteManager->getSprite(id);
}

void World::unloadAllSprites() {
    return mSpriteManager->unloadAll();
}

AnimId World::loadAnim(std::string name, const char* path, int frameCount, int fps) {
    return mAnimationManager->loadAnim(name, path, frameCount, fps);
}

AnimationClip& World::getAnim(AnimId id) {
    return mAnimationManager->getAnim(id);
}

void World::unloadAllAnims() {
    return mAnimationManager->unloadAll();
}

void World::save(const char* path) {
    Writer w;
    w.file = fopen(path, "wb");
    if (!w.file) return;

    // count living entities first
    uint32_t count = 0;
    for (Entity e = 0; e < MAX_ENTITIES; ++e)
        if (mEntityManager->getSignature(e).any()) count++;

    w.write(count);

    for (Entity e = 0; e < MAX_ENTITIES; ++e) {
        Signature sig = mEntityManager->getSignature(e);
        if (sig.none()) continue;

        w.write(e);
        w.write(sig);

        for (uint8_t i = 0; i < MAX_COMPONENTS; ++i) {
            if (sig.test(i) && mSerializers[i].serialize)
                mSerializers[i].serialize(w, mSerializers[i].arrayPtr, e);
        }
    }

    fclose(w.file);
}

void World::load(const char* path) {
    Reader r;
    r.file = fopen(path, "rb");
    if (!r.file) return;

    uint32_t count = 0;
    r.read(count);

    for (uint32_t i = 0; i < count; ++i) {
        Entity e;
        Signature sig;
        r.read(e);
        r.read(sig);

        mEntityManager->restoreEntity(e, sig);

        for (uint8_t j = 0; j < MAX_COMPONENTS; ++j) {
            if (sig.test(j) && mSerializers[j].deserialize)
                mSerializers[j].deserialize(r, mSerializers[j].arrayPtr, e);
        }

        mSystemManager->entitySignatureChanged(e, sig);
    }

    mEntityManager->rebuildAvailableQueue();

    fclose(r.file);
}