#pragma once

#include "System.h"
#include "World.h"
#include "Components.h"
#include <raylib.h>


class PhysicsSystem : public System {
	public:
		World* world = nullptr;

		void update(float dt) override;
		void updateCollisions(float dt, bool drawBounds);
		bool isColliding(Entity a, Entity b) const;

		void drawDebug();
};