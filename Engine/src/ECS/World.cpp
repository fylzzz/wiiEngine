#include "World.h"
#include "Path.h"
#include "System.h"
#include "EngineComponents.h"


void World::init() {
	mEntityManager = std::make_unique<EntityManager>();
	mComponentManager = std::make_unique<ComponentManager>();
	mSystemManager = std::make_unique<SystemManager>();
	mResourceManager = std::make_unique<ResourceManager>();
    mSpriteManager = std::make_unique<SpriteManager>();
    mAnimationManager = std::make_unique<AnimationManager>();
    mAudioManager = std::make_unique<AudioManager>();

    RegisterEngineComponents(*this);
}

Entity World::createEntity() {
	return mEntityManager->create();
}

void World::destroyEntity(Entity e) {
	mEntityManager->destroy(e);
	mComponentManager->entityDestroyed(e);
	mSystemManager->entityDestroyed(e);
}

void World::destroyAllEntities() {
    for (Entity e = 0; e < MAX_ENTITIES; ++e) {
        if (mEntityManager->getSignature(e).any()) {
            destroyEntity(e);
        }
    }
}

ResourceId World::loadModel(const char* path) {
	return mResourceManager->loadModel(path);
}

Model& World::getModel(ResourceId id) {
	return mResourceManager->getModel(id);
}

BoundingBox& World::getModelBoundingBox(ResourceId id) {
    return mResourceManager->getModelBoundingBox(id);
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

Texture2D& World::getSpriteTexture(SpriteId id) {
    return mSpriteManager->getTexture(id);
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

AudioId World::loadAudio(std::string name, const char* path) {
    return mAudioManager->loadAudio(name, path);
}

int World::playAudio(AudioId id, int volL, int volR) {
    return mAudioManager->playAudio(id, volL, volR);
}

bool World::save(const char* path) {
    Writer w(ResolvePath(path).c_str());
    if (!w.ok()) return false;

    w.u32(SCENE_MAGIC);
    w.u16(SCENE_VER);

    // ---- asset table ----
    w.u16(mResourceManager->count());
    for (uint16_t i = 0; i < mResourceManager->count(); ++i)
        w.str(mResourceManager->path(i));

    w.u16(mSpriteManager->count());
    for (uint16_t i = 0; i < mSpriteManager->count(); ++i)
        w.str(mSpriteManager->path(i));

    w.u16(mAnimationManager->count());
    for (uint16_t i = 0; i < mAnimationManager->count(); ++i) {
        const AnimationClip& a = mAnimationManager->getAnim(i);
        w.str(a.name);
        w.str(mAnimationManager->path(i));
        w.u16(uint16_t(a.frameCount));
        w.u16(uint16_t(a.fps));
    }

    // Only serialize components that are registered with the serializer
    Signature savable;
    for (uint8_t i = 0; i < MAX_COMPONENTS; ++i) {
        if (mSerializers[i].serialize) savable.set(i);
    }

    uint32_t count = 0;
    for (Entity e = 0; e < MAX_ENTITIES; ++e) {
        if (mEntityManager->getSignature(e).any()) ++count;
    }
    w.u32(count);

    for (Entity e = 0; e < MAX_ENTITIES; ++e) {
        Signature sig = mEntityManager->getSignature(e);
        if (sig.none()) continue;

        Signature out = sig & savable;
        w.u32(e);
        w.u32(uint32_t(out.to_ulong()));

        for (uint8_t i = 0; i < MAX_COMPONENTS; ++i) {
            if (out.test(i)) {
                mSerializers[i].serialize(w, mSerializers[i].arrayPtr, e);
            }
        }
    }

    return w.ok();
}

bool World::load(const char* path) {
    Reader r(ResolvePath(path).c_str());
    if (!r.ok()) return false;

    if (r.u32() != SCENE_MAGIC)   return false;
    if (r.u16() != SCENE_VER) return false;

    // ---- asset table: load each, record its new ID ----
    AssetMap map;

    uint16_t n = r.u16();
    for (uint16_t i = 0; i < n && r.ok(); ++i) {
        std::string p = r.str();
        map.models.push_back(loadModel(p.c_str()));
    }

    n = r.u16();
    for (uint16_t i = 0; i < n && r.ok(); ++i) {
        std::string p = r.str();
        map.sprites.push_back(loadSprite(p.c_str()));
    }

    n = r.u16();
    for (uint16_t i = 0; i < n && r.ok(); ++i) {
        std::string name = r.str();
        std::string p = r.str();
        int frames = r.u16();
        int fps = r.u16();
        map.anims.push_back(loadAnim(name, p.c_str(), frames, fps));
    }
    if (!r.ok()) return false;

    r.map = &map;

    uint32_t count = r.u32();
    if (!r.ok() || count > MAX_ENTITIES) return false;

    for (uint32_t i = 0; i < count; ++i) {
        Entity e = r.u32();
        Signature sig(r.u32());
        if (!r.ok() || e >= MAX_ENTITIES) return false;

        mEntityManager->restoreEntity(e, sig);

        for (uint8_t j = 0; j < MAX_COMPONENTS; ++j) {
            if (!sig.test(j)) continue;
            if (!mSerializers[j].deserialize) return false;   // component not registered
            mSerializers[j].deserialize(r, mSerializers[j].arrayPtr, e);
        }
        if (!r.ok()) return false;

        mSystemManager->entitySignatureChanged(e, sig);
        ApplyComponentMethods(*this, e, sig);
    }

    mEntityManager->rebuildAvailableQueue();
    return r.ok();
}