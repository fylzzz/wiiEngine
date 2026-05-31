#pragma once

#include "Types.h"

#include <array>
#include <cassert>
#include <queue>

class EntityManager {
public:
	EntityManager();

	Entity create();
	void destroy(Entity e);
	void setSignature(Entity e, Signature sig);
	Signature getSignature(Entity e) const;

	void restoreEntity(Entity e, Signature sig);
	void rebuildAvailableQueue();

private:
	std::queue<Entity> mAvailable;
	std::array<Signature, MAX_ENTITIES> mSignatures{};
	uint32_t mLiving = 0;
};