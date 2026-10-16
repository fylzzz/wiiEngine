#include "AudioManager.h"
#include <asndlib.h>
#include <malloc.h>
#include <string.h>
#include <ogc/cache.h>
#include <math.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


static inline size_t AlignedWaveBytes(const Wave& w) {
	size_t raw = (size_t)w.frameCount * w.channels * (w.sampleSize / 8);
	return (raw + 31) & ~(size_t)31;   // ASND/DMA: multiple of 32
}


AudioId AudioManager::loadAudio(std::string name, const char* path) {
	for (AudioId i = 0; i < mSize; ++i) {
		// check that audio hasn't been loaded, otherwise return that path
		if (mPaths[i] && strcmp(mPaths[i], path) == 0) {
			return i;
		}
	}
	
	AudioId id = mSize++;
	mAudio[id].name = name;

	Wave w = { };
	unsigned int ch, rate;
	drwav_uint64 frames;

	int16_t* pcm = drwav_open_file_and_read_pcm_frames_s16(path, &ch, &rate, &frames, nullptr);
	if (!pcm) return INVALID_AUDIO;

	w.frameCount = (unsigned int)frames;
	w.sampleRate = rate;
	w.sampleSize = 16;
	w.channels = ch;

	size_t raw = (size_t)frames * ch * sizeof(int16_t);
	size_t bytes = AlignedWaveBytes(w);
	w.data = memalign(32, bytes);
	std::memset(w.data, 0, bytes);
	std::memcpy(w.data, pcm, raw);
	drwav_free(pcm, nullptr);

	DCFlushRange(w.data, bytes);
	mAudio[id].audioData = w;
	return id;
}

int AudioManager::playAudio(AudioId id, int volL, int volR) {
	int voice = ASND_GetFirstUnusedVoice();
	if (voice < 0) return -1;
	s32 fmt = (mAudio[id].audioData.channels == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
	ASND_SetVoice(voice, fmt, mAudio[id].audioData.sampleRate, 0, mAudio[id].audioData.data, AlignedWaveBytes(mAudio[id].audioData), volL, volR, nullptr);
	return voice;

	/*const int rate = 48000, frames = rate;
	size_t bytes = (frames * sizeof(int16_t) + 31) & ~31;
	int16_t* tone = (int16_t*)memalign(32, bytes);
	std::memset(tone, 0, bytes);
	for (int i = 0; i < frames; ++i)
		tone[i] = (int16_t)(sinf(2.0f * 3.14159265f * 440.0f * i / rate) * 12000.0f);
	DCFlushRange(tone, bytes);

	ASND_SetVoice(voice, VOICE_MONO_16BIT, rate, 0, tone, bytes, volL, volR, nullptr);
	return voice;*/
}