#pragma once

#include "raylib.h"
#include "Types.h"

#include <array>
#include <vector>
#include <cstring>


struct AnimationClip {
	std::string name;
	Texture2D spritesheet;
	std::vector<Rectangle> frames;
	int frameCount;
	int fps;
	bool loop = true;
};

using AnimId = uint16_t;
constexpr AnimId INVALID_ANIM = 0xFFFF;
constexpr AnimId MAX_ANIMS = 256;

class AnimationManager {
	public:
		AnimId loadAnim(std::string name, const char* path, int frameCount, int fps);
		AnimationClip& getAnim(AnimId id);
		void unloadAll();

	private:
		std::array<AnimationClip, MAX_ANIMS> mAnims{};
		std::array<const char*, MAX_ANIMS> mPaths{};
		uint16_t mSize = 0;
};