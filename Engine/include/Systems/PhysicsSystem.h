#pragma once

#include "System.h"
#include "World.h"
#include "Components.h"
#include <raylib.h>


class PhysicsRay {
	public:
		PhysicsRay(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction)
		{
			invDirection = Vector3{
				direction.x != 0.0f ? 1.0f / direction.x : 1e30f,
				direction.y != 0.0f ? 1.0f / direction.y : 1e30f,
				direction.z != 0.0f ? 1.0f / direction.z : 1e30f
			};
			sign[0] = (invDirection.x < 0);
			sign[1] = (invDirection.y< 0);
			sign[2] = (invDirection.z < 0);
		}

		Vector3 origin, direction;
		Vector3 invDirection;
		int sign[3];
};

class PhysicsSystem : public System {
	public:
		World* world = nullptr;

		void update(float dt) override;
		void updateCollisions(float dt, bool drawBounds);
		bool isColliding(Entity a, Entity b) const;

		bool rayTest(const PhysicsRay& r) const;

		void drawDebug();
};