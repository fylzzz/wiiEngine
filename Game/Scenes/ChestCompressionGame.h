#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <wiiuse/wpad.h>
#include <math.h>
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <array>


class ChestCompressionGame : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	//accel
	float ax, ay, az;
	//timer
	float timer = 0.0f;
	float bpmTimer = 0.0f; 
	float gameDuration = 60.0f;
	float refractoryTimer = 0.0f;
	float lastFlickTimeStamp = -1.0f; //-1 for no previous flick 

	//BPM 
	float currentBPM = 0.0f;
	float targetBPM = 120.0f; 
	float bpmWindow = 10.0f; 
	float compressionThresh = 600.0f;
	float refractory = 0.3f; //min seconds between flicks 

	//average roll
	int BPMsampleCount = 4;
	std::array<float, 4> flickIntervals = {0.0f, 0.0f, 0.0f, 0.0f};
	int intervalIndex = 0;
	int intervalsFilled = 0;

	bool isFlicking = false;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		timer = gameDuration;

		// Setup camera
		camera.position = Vector3{ 0.0f, 5.0f, 5.0f };
		camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
		camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
		camera.fovy = 90.0f;
		camera.projection = CAMERA_PERSPECTIVE;


		// Setup components and systems
		world.registerComponent<EngineTransform>();
		world.registerComponentSerializer<EngineTransform>();
		world.registerComponent<Renderable>();
		world.registerComponentSerializer<Renderable>();
		world.registerComponent<Renderable2D>();
		world.registerComponentSerializer<Renderable2D>();
		world.registerComponent<Collider2D>();
		world.registerComponentSerializer<Collider2D>();
		world.registerComponent<RigidBody2D>();
		world.registerComponentSerializer<RigidBody2D>();
		world.registerComponent<Animator2D>();
		world.registerComponentSerializer<Animator2D>();

		rendersys = world.registerSystem<RenderSystem>();
		rendersys->world = &world;
		rendersys->camera = &camera;
		physics = world.registerSystem<PhysicsSystem>();
		physics->world = &world;
		animation = world.registerSystem<AnimationSystem>();
		animation->world = &world;

		Signature renderSig;
		renderSig.set(world.getComponentType<EngineTransform>());
		world.setSystemSignature<RenderSystem>(renderSig);

		Signature physicsSig;
		physicsSig.set(world.getComponentType<EngineTransform>());
		physicsSig.set(world.getComponentType<Collider2D>());
		physicsSig.set(world.getComponentType<RigidBody2D>());
		world.setSystemSignature<PhysicsSystem>(physicsSig);

		Signature animationSig;
		animationSig.set(world.getComponentType<EngineTransform>());
		animationSig.set(world.getComponentType<Animator2D>());
		world.setSystemSignature<AnimationSystem>(animationSig);


		// Load entities from file
		FILE* f = fopen("sd:/samplescene.bin", "rb");
		if (f) 
		{
			fclose(f);
			world.load("sd:/samplescene.bin");
		}
		else
		{
			// Create new/default entities here
			return;
		}
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		if (timer > 0.0f)
		{
			timer -= dt;
			if (timer < 0.0f) timer = 0.0f;
		}
			refractoryTimer += dt;

		if (data->data_present) {
			az = data->accel.z;
			ax = data->accel.x;
			ay = data->accel.y;

			bool flicking = (float) az > 560.0f;

			if (flicking && !isFlicking && refractoryTimer >= refractory)
			{
				CompressionBPM();
				refractoryTimer = 0.0f;
				isFlicking = true;
			}
			else if (!flicking)
			{
				isFlicking = false;  // <-- reset once accel drops back down
			}
		}

		if (refractoryTimer > 3.0f)
		{
			currentBPM = 0.0f;
			lastFlickTimeStamp = -1.0f;
			intervalIndex = 0;
			intervalsFilled = 0;
		}

		
		// update physics system
		physics->update(dt);
		physics->updateCollisions(dt, false);

		// update animation system
		animation->update(dt);
	}

	void CompressionBPM()
	{
		float now = gameDuration - timer;
		
		if (lastFlickTimeStamp < 0.0f)
		{
			lastFlickTimeStamp = now;
			return;
		}
		
		float timeSinceLast = now - lastFlickTimeStamp;
		lastFlickTimeStamp = now;

		//ignore flick if too early 
		if (timeSinceLast <= 0.0f || timeSinceLast > 2.0f) return;

		//store intervals in a ring buffer
		flickIntervals[intervalIndex] = timeSinceLast;
		intervalIndex = (intervalIndex + 1) % BPMsampleCount;
		if (intervalsFilled < BPMsampleCount) intervalsFilled++;

		float total = 0.0f; 
		for (int i = 0; i < intervalsFilled; i++)
		{
		total += flickIntervals[i];
		}

		float avgInterval = total/intervalsFilled;
		currentBPM = 60.0f / avgInterval;	 
	
		if (std::abs(currentBPM - targetBPM) <= bpmWindow)
		{

		}
	
		else if (currentBPM < targetBPM)
		{

		}
	
		else 
		{

		}
	}
	void render(float dt) override {
		BeginDrawing();
		ClearBackground(BLACK);
		glClear(GL_DEPTH_BUFFER_BIT);
		rendersys->update(dt);

		DrawText(TextFormat("ACCEL: %.0f, %.0f, %.0f", ax, ay, az), 10, 30, 20, WHITE);
		DrawText(TextFormat("Timer: %.0f", timer), 10, 50, 20, WHITE);
		DrawText(TextFormat("CurrentBPM: %f", currentBPM), 10, 70, 20, WHITE);
		DrawText(TextFormat("TargetBPM: %f", targetBPM), 10, 90, 20, WHITE);
			
		DrawFPS(10, 10);
		EndDrawing();
	}

	void shutdown() override {
		// save current scene state
		world.save("sd:/samplescene.bin");
		// unload scene resources
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};