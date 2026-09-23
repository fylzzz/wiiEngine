#pragma once

#include "System.h"
#include "World.h"
#include "Components.h"
#include <raylib.h>
#include "raymath.h"


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

        // Draw a thick 3D "line" from startPos to endPos with a given thickness (radius)
        static void DrawRay(Vector3 startPos, Vector3 endPos, float thickness, Color color)
        {
            Vector3 direction = Vector3Subtract(endPos, startPos);
            float length = Vector3Length(direction);

            // Find midpoint for cylinder position
            Vector3 midPos = {
                startPos.x + direction.x / 2.0f,
                startPos.y + direction.y / 2.0f,
                startPos.z + direction.z / 2.0f
            };

            // Calculate rotation to align cylinder with the direction vector
            // Default cylinder is drawn along the Y-axis (0, 1, 0)
            Vector3 up = { 0.0f, 1.0f, 0.0f };
            Vector3 axis = Vector3CrossProduct(up, Vector3Normalize(direction));
            float angle = 0.0f;

            if (Vector3Length(axis) > 0.00001f) {
                angle = RAD2DEG * acosf(Vector3DotProduct(up, Vector3Normalize(direction)));
            }
            else {
                // Handle straight up/down edge cases
                if (direction.y < 0) {
                    angle = 180.0f;
                    axis = (Vector3){ 1.0f, 0.0f, 0.0f };
                }
                else {
                    axis = (Vector3){ 0.0f, 0.0f, 1.0f };
                }
            }

            // Draw the cylinder (radius top/bottom = thickness / 2)
            DrawCylinderEx(startPos, endPos, thickness / 2.0f, thickness / 2.0f, 8, color);
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
		Entity getCollision(Entity test) const;

		Entity* rayTest(const PhysicsRay& r) const;

		void drawDebug();
	private:
		mutable Entity mLastHitEntity = 0;
};