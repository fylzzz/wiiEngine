#pragma once

#include <asndlib.h>
#include "dr_wav.h"


class MusicStream {
	public:
		bool loadTrack(const char* path, bool loop = true);
		void playTrack(int volL, int volR);
		void update();
		void stopTrack();
		void unloadTrack();
		bool isPlaying() const { return mVoice >= 0; };


		int dbgUpdates = 0;     // update() calls while playing
		int dbgFills = 0;       // buffers queued from update()
		s32 dbgAddRet = 99;     // last ASND_AddVoice return
		s32 dbgPrimeRet = 99;   // AddVoice return in playTrack
		s32 voice() const { return mVoice; }


	private:
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
};