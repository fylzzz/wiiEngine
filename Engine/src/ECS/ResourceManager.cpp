#include "ResourceManager.h"
#include "Path.h"


ResourceId ResourceManager::loadModel(const char* path) {
	for (ResourceId i = 0; i < mSize; ++i) {
		if (mPaths[i] == path) return i;
	}

	ResourceId id = mSize++;
	mModels[id] = LoadModel(ResolvePath(path).c_str());
	mBoundingBoxes[id] = GetModelBoundingBox(mModels[id]);
	mPaths[id] = path;
	return id;
}


Model& ResourceManager::getModel(ResourceId id) {
	return mModels[id];
}

void ResourceManager::unloadAll() {
	for (ResourceId i = 0; i < mSize; ++i) {
		UnloadModel(mModels[i]);
		mPaths[i].clear();
	}
	mSize = 0;
}