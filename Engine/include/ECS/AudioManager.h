#pragma once

#include "raylib.h"
#include "Types.h"

#include <vector>
#include <array>
#include <cstring>
#include <string>


struct AudioSample {
	std::string name;
	Wave audioData;

#ifndef GEKKO
	Sound sound{};
#endif
};


using AudioId = uint16_t;
constexpr AudioId INVALID_AUDIO = 0xFFFF;
constexpr AudioId MAX_AUDIO = 256;

class AudioManager {
	public:
		uint16_t count() const { return mSize; }
		const std::string& path(AudioId id) const { return mPaths[id]; }

		AudioId loadAudio(std::string name, const char* path);
		int playAudio(AudioId id, int volL, int volR);

	private:
		std::array<AudioSample, MAX_AUDIO> mAudio;
		std::array<std::string, MAX_AUDIO> mPaths;
		uint16_t mSize = 0;
};