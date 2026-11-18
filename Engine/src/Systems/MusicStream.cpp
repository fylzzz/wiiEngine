#include "MusicStream.h"


#ifdef GEKKO

#include <malloc.h>
#include <cstring>
#include <ogc/cache.h>


static void MusicVoiceCallback(s32 voice) {}


bool MusicStream::loadTrack(const char* path, bool loop) {
	unloadTrack();
	if (!drwav_init_file(&mWav, path, nullptr)) return false;

	if (mWav.translatedFormatTag != DR_WAVE_FORMAT_PCM || mWav.bitsPerSample != 16) {
		drwav_uninit(&mWav);
		return false;
	}

	mOpen = true;
	mLoop = loop;
	mEnded = false;
	mBufBytes = FRAMES_PER_BUF * mWav.channels * sizeof(int16_t);
	for (auto& b : mBufs) {
		b = (int16_t*)memalign(32, mBufBytes);
	}
	return true;
}

bool MusicStream::fill(int i) {
	if (mEnded) return false;

	int16_t* dst = mBufs[i];
	size_t got = drwav_read_pcm_frames_s16(&mWav, FRAMES_PER_BUF, dst);

	while (got < FRAMES_PER_BUF && mLoop) {
		drwav_seek_to_pcm_frame(&mWav, 0);
		got += drwav_read_pcm_frames_s16(&mWav, FRAMES_PER_BUF - got, dst + got * mWav.channels);
	}

	if (got < FRAMES_PER_BUF) {
		std::memset(dst + got * mWav.channels, 0, (FRAMES_PER_BUF - got) * mWav.channels * sizeof(int16_t));
		mEnded = true;
	}

	DCFlushRange(dst, mBufBytes);
	return true;
}

void MusicStream::playTrack(int volL, int volR) {
	if (!mOpen) return;
	stopTrack();
	drwav_seek_to_pcm_frame(&mWav, 0);
	mEnded = false;

	mVoice = ASND_GetFirstUnusedVoice();
	if (mVoice < 0) return;

	s32 fmt = (mWav.channels == 2) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT;
	fill(0);
	if (ASND_SetVoice(mVoice, fmt, mWav.sampleRate, 0, mBufs[0], mBufBytes,
		volL, volR, MusicVoiceCallback) != SND_OK) {
		mVoice = -1;
		return;
	}

	mLastQueued = 0;
	mNext = 1;
	mPending = false;
}

void MusicStream::update() {
	if (mVoice < 0) return;
	dbgUpdates++;

	if (!mPending && !mEnded) {
		fill(mNext);
		mPending = true;
	}

	if (mPending && ASND_TestVoiceBufferReady(mVoice) == SND_OK) {
		dbgAddRet = ASND_AddVoice(mVoice, mBufs[mNext], mBufBytes);
		if (dbgAddRet == SND_OK) {
			dbgFills++;
			mLastQueued = mNext;
			mPending = false;
			mNext = (mNext + 1) % NUM_BUFS;
		}
	}

	if (mEnded && !mPending && ASND_TestPointer(mVoice, mBufs[mLastQueued]) == SND_OK) {
		stopTrack();
	}
}

void MusicStream::stopTrack() {
	if (mVoice >= 0) ASND_StopVoice(mVoice);
	mVoice = -1;
}

void MusicStream::unloadTrack() {
	stopTrack();
	if (mOpen) drwav_uninit(&mWav);
	for (auto& b : mBufs) {
		free(b);
		b = nullptr;
	}
	mOpen = false;
}

#else

#include <algorithm>


bool MusicStream::loadTrack(const char* path, bool loop) {
	unloadTrack();
	mMusic = LoadMusicStream(path);
	if (!IsMusicValid(mMusic)) return false;

	mMusic.looping = loop;
	mLoop = loop;
	mLoaded = true;
	return true;
}

void MusicStream::playTrack(int volL, int volR) {
	if (!mLoaded) return;

	int peak = std::max(volL, volR);
	SetMusicVolume(mMusic, peak / 255.0f);
	float pan = peak > 0 ? 0.5f + 0.5f * float(volL - volR) / float(peak) : 0.5f;
	SetMusicPan(mMusic, pan);
	PlayMusicStream(mMusic);
	mPlaying = true;
}

void MusicStream::update() {
	if (!mPlaying) return;
	++dbgUpdates;
	UpdateMusicStream(mMusic);
	if (!mLoop && !IsMusicStreamPlaying(mMusic)) {
		mPlaying = false;
	}
}

void MusicStream::stopTrack() {
	if (mLoaded) StopMusicStream(mMusic);
	mPlaying = false;
}

void MusicStream::unloadTrack() {
	stopTrack();
	if (mLoaded) UnloadMusicStream(mMusic);
	mLoaded = false;
}
#endif