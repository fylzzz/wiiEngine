#include "ResourceManager.h"
#include "Path.h"


ResourceId ResourceManager::loadModel(const char* path) {
	for (ResourceId i = 0; i < mSize; ++i) {
		// check that model hasn't already been loaded, if it has then return that path
		if (mPaths[i] && strcmp(mPaths[i], path) == 0) {
			return i;
		}
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
		mSize = 0;
	}
}