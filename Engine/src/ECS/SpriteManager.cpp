#include "SpriteManager.h"


SpriteId SpriteManager::loadSprite(const char* path) {
	for (SpriteId i = 0; i < mSize; ++i) {
		// check that sprite hasn't already been loaded, if it has then return that path
		if (mPaths[i] && strcmp(mPaths[i], path) == 0) {
			return i;
		}
	}

	SpriteId id = mSize++;
	mSprites[id] = LoadImage(path);
	mPaths[id] = path;
	return id;
}

Image& SpriteManager::getSprite(SpriteId id) {
	return mSprites[id];
}

void SpriteManager::unloadAll() {
	for (SpriteId i = 0; i < mSize; ++i) {
		UnloadImage(mSprites[i]);
		mSize = 0;
	}
}