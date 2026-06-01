#pragma once

#include "raylib.h"
#include "Types.h"

#include <array>
#include <cstring>


using SpriteId = uint16_t;
constexpr SpriteId INVALID_SPRITE = 0xFFFF;
constexpr SpriteId MAX_SPRITES = 256;

class SpriteManager {
public:
	SpriteId loadSprite(const char* path);
	Image& getSprite(SpriteId id);
	void unloadAll();

private:
	std::array<Image, MAX_SPRITES> mSprites{};
	std::array<const char*, MAX_SPRITES> mPaths{};
	uint16_t mSize = 0;
};