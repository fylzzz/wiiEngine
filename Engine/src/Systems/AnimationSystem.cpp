#include "AnimationSystem.h"


void AnimationSystem::update(float dt) {
	for (Entity e : mEntities) {
		auto& anim = world->getComponent<Animator2D>(e);

		auto it = anim.animClips.find(anim.currentAnim);
		if (it == anim.animClips.end() || it->second == INVALID_ANIM) continue;

		AnimationClip& clip = world->getAnim(it->second);
		if (clip.frames.empty()) continue;

		anim.frameTimer += dt;
		float frameDuration = 1.0f / clip.fps;
		if (anim.frameTimer >= frameDuration) {
			anim.frameTimer -= frameDuration;
			int nextFrame = anim.currentFrame + 1;

			if (nextFrame >= (int)clip.frames.size()) {
				if (clip.loop) {
					anim.currentFrame = 0;
				}
				else {
					anim.currentFrame = (int)clip.frames.size() - 1;
					anim.finished = true;
				}
			}
			else {
				anim.currentFrame = nextFrame;
				anim.finished = false;
			}
		}
	}
}

void AnimationSystem::setAnim(Animator2D& anim, const std::string& name) {
	if (anim.currentAnim == name) return; // already playing, don't restart
	anim.currentAnim = name;
	anim.currentFrame = 0;
	anim.frameTimer = 0.0f;
	anim.finished = false;
}