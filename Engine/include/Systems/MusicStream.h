#pragma once

#include "raylib.h"
#include <cstdint>
#include <cstddef>

#ifdef GEKKO

#include <asndlib.h>
#include "dr_wav.h"

#endif


class MusicStream {
	public:
		bool loadTrack(const char* path, bool loop = true);
		void playTrack(int volL, int volR);
		void update();
		void stopTrack();
		void unloadTrack();
		
#ifdef GEKKO
		bool isPlaying() const { return mVoice >= 0; };
		int32_t voice() const { return mVoice; };
#else
		bool isPlaying() const { return mPlaying; };
		int32_t voice() const { return mPlaying ? 0 : -1; };
#endif


		int dbgUpdates = 0;     // update() calls while playing
		int dbgFills = 0;       // buffers queued from update()
		int32_t dbgAddRet = 99;     // last ASND_AddVoice return
		int32_t dbgPrimeRet = 99;   // AddVoice return in playTrack


	private:
#ifdef GEKKO
		static constexpr int NUM_BUFS = 3;
		static constexpr size_t FRAMES_PER_BUF = 16384;

		bool fill(int i);

		drwav mWav = {};
		bool mOpen = false;
		bool mLoop = true;
		bool mEnded = false;
		int16_t* mBufs[NUM_BUFS] = {};
		size_t mBufBytes = 0;
		int  mNext = 0;
		int  mLastQueued = 0;
		bool mPending = false;
		s32 mVoice = -1;
#else
		Music mMusic{};
		bool mLoaded = false;
		bool mPlaying = false;
		bool mLoop = true;
#endif
};