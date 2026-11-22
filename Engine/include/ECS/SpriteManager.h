#pragma once

#include "raylib.h"
#include "Types.h"

#include <array>
#include <cstring>
#include <string>


using SpriteId = uint16_t;
constexpr SpriteId INVALID_SPRITE = 0xFFFF;
constexpr SpriteId MAX_SPRITES = 256;

class SpriteManager {
public:
	uint16_t count() const { return mSize; }
	const std::string path(SpriteId id) const { return mPaths[id]; }

	SpriteId loadSprite(const char* path);
	Image& getSprite(SpriteId id);
	void unloadAll();

	Texture2D& getTexture(SpriteId id);

private:
	std::array<Image, MAX_SPRITES> mSprites{};
	std::array<Texture2D, MAX_SPRITES> mTextures{};
	std::array<std::string, MAX_SPRITES> mPaths{};
	uint16_t mSize = 0;
};