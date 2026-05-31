#pragma once

#include <raylib.h>
#include "Types.h"

#include <array>
#include <cstring>


using ResourceId = uint16_t;
constexpr ResourceId INVALID_RESOURCE = 0xFFFF;
constexpr ResourceId MAX_RESOURCES = 256;

class ResourceManager {
	public:
		ResourceId loadModel(const char* path);
		Model& getModel(ResourceId id);
		void unloadAll();

	private:
		std::array<Model, MAX_RESOURCES> mModels{};
		std::array<const char*, MAX_RESOURCES> mPaths{};
		uint16_t mSize = 0;
};