#pragma once

#include <raylib.h>
#include "Types.h"

#include <array>
#include <cstring>
#include <string>


using ResourceId = uint16_t;
constexpr ResourceId INVALID_RESOURCE = 0xFFFF;
constexpr ResourceId MAX_RESOURCES = 256;

class ResourceManager {
	public:
		uint16_t count() const { return mSize; }
		const std::string& path(ResourceId id) const { return mPaths[id]; }

		ResourceId loadModel(const char* path);
		Model& getModel(ResourceId id);
		void unloadAll();

		BoundingBox& getModelBoundingBox(ResourceId id) { return mBoundingBoxes[id]; }

	private:
		std::array<Model, MAX_RESOURCES> mModels{};
		std::array<BoundingBox, MAX_RESOURCES> mBoundingBoxes{};
		std::array<std::string, MAX_RESOURCES> mPaths{};
		uint16_t mSize = 0;
};