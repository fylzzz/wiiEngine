#pragma once

#include "raylib.h"
#include "Types.h"

#include <vector>
#include <array>
#include <cstring>


struct AudioSample {
	std::string name;
	Wave audioData;
};


using AudioId = uint16_t;
constexpr AudioId INVALID_AUDIO = 0xFFFF;
constexpr AudioId MAX_AUDIO = 256;

class AudioManager {
	public:
		AudioId loadAudio(std::string name, const char* path);
		int playAudio(AudioId id, int volL, int volR);

	private:
		std::array<AudioSample, MAX_AUDIO> mAudio;
		std::array<const char*, MAX_AUDIO> mPaths;
		uint16_t mSize = 0;
};