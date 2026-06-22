#include "AnimationManager.h"


AnimId AnimationManager::loadAnim(std::string name, const char* path, int frameCount, int fps) {
	for (AnimId i = 0; i < mSize; ++i) {
		// check that anim hasn't already been loaded, if it has then return that path
		if (mPaths[i] && strcmp(mPaths[i], path) == 0) {
			return i;
		}
	}

	AnimId id = mSize++;
	mAnims[id].name = name;
	Image img = LoadImage(path);
	mAnims[id].spritesheet = LoadTextureFromImage(img);
	UnloadImage(img);
	mAnims[id].frameCount = frameCount;
	mAnims[id].fps = fps;

	mPaths[id] = path;

	int frameWidth = mAnims[id].spritesheet.width / frameCount;
	int frameHeight = mAnims[id].spritesheet.height;
	mAnims[id].frames.reserve(frameCount);
	for (int i = 0; i < frameCount; ++i) {
		mAnims[id].frames.push_back((Rectangle){ (float)(i * frameWidth), 0.0f, (float)frameWidth, (float)frameHeight });
	}

	return id;
}

AnimationClip& AnimationManager::getAnim(AnimId id) {
	return mAnims[id];
}

void AnimationManager::unloadAll() {
	for (AnimId i = 0; i < mSize; ++i) {
		UnloadTexture(mAnims[i].spritesheet);
	}
	mSize = 0;
}