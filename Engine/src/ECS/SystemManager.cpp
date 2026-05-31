#include "SystemManager.h"


void SystemManager::entityDestroyed(Entity e) {
	for (auto& sys : mSystems) {
		if (sys) {
			sys->mEntities.erase(e);
		}
	}
}

void SystemManager::entitySignatureChanged(Entity e, Signature sig) {
	for (size_t i = 0; i < MAX_SYSTEMS; ++i) {
		if (!mSystems[i]) continue;
		if ((sig & mSignatures[i]) == mSignatures[i]) {
			mSystems[i]->mEntities.insert(e);
		}
		else {
			mSystems[i]->mEntities.erase(e);
		}
	}
}