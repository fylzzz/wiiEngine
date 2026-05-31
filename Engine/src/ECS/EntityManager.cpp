#include "EntityManager.h"


EntityManager::EntityManager() {
	// reserve memory for entity array
	for (Entity e = 0; e < MAX_ENTITIES; ++e) {
		mAvailable.push(e);
	}
}

Entity EntityManager::create() {
	// ensure we don't have too many entities
	assert(mLiving < MAX_ENTITIES && "Too many entities");
	// assign new entity id from front of free slot queue and shrink
	Entity id = mAvailable.front();
	mAvailable.pop();
	++mLiving;
	return id;
}

void EntityManager::destroy(Entity e) {
	// remove signatures of attached components
	mSignatures[e].reset();
	// return slot to free entities array
	mAvailable.push(e);
	--mLiving;
}

void EntityManager::setSignature(Entity e, std::bitset<32> sig) {
	// add component signature
	mSignatures[e] = sig;
}

Signature EntityManager::getSignature(Entity e) const {
	return mSignatures[e];
}

void EntityManager::restoreEntity(Entity e, Signature sig) {
	mSignatures[e] = sig;
	++mLiving;
	// note: does not remove e from mAvailableEntities
	// call rebuildAvailableQueue() after loading all entities
}

void EntityManager::rebuildAvailableQueue() {
	// clear and rebuild from scratch
	while (!mAvailable.empty()) mAvailable.pop();
	for (Entity e = 0; e < MAX_ENTITIES; ++e)
		if (mSignatures[e].none()) mAvailable.push(e);
}
