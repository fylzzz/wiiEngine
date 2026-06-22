#pragma once

#include "System.h"
#include "World.h"
#include "Components.h"


class AnimationSystem : public System {
	public:
		World* world = nullptr;

		void update(float dt) override;
		void setAnim(Animator2D& anim, const std::string& name);
};