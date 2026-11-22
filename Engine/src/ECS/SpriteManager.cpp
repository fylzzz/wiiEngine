#include "SpriteManager.h"
#include "Path.h"


SpriteId SpriteManager::loadSprite(const char* path) {
	for (SpriteId i = 0; i < mSize; ++i) {
		// check that sprite hasn't already been loaded, if it has then return that path
		if (mPaths[i] == path) {
			return i;
		}
	}

	SpriteId id = mSize++;
	mSprites[id] = LoadImage(ResolvePath(path).c_str());
	mPaths[id] = path;
	return id;
}

Image& SpriteManager::getSprite(SpriteId id) {
	return mSprites[id];
}

Texture2D& SpriteManager::getTexture(SpriteId id) {
	mTextures[id] = LoadTextureFromImage(mSprites[id]);
	return mTextures[id];
}

void SpriteManager::unloadAll() {
	for (SpriteId i = 0; i < mSize; ++i) {
		UnloadTexture(mTextures[i]);
		UnloadImage(mSprites[i]);
		mPaths[i].clear();
	}
	mSize = 0;
}