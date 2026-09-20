#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <wiiuse/wpad.h>
#include <math.h>

#include <raymath.h>


class PhysicsTest : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	bool board;
	struct wii_board_t* wb;

	float total, x, y, screenX, screenY;

	Entity e;
	Entity other;
	ResourceId teapotId;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		teapotId = world.loadModel("sd:/sphere.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		// Setup camera
		camera.position = Vector3{ 0.0f, 5.0f, 10.0f };
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
		world.registerComponent<PrimitiveRenderable2D>();
		world.registerComponentSerializer<PrimitiveRenderable2D>();
		world.registerComponent<Collider2D>();
		world.registerComponentSerializer<Collider2D>();
		world.registerComponent<BoxCollider>();
		world.registerComponentSerializer<BoxCollider>();
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
		//physicsSig.set(world.getComponentType<Collider2D>());
		physicsSig.set(world.getComponentType<BoxCollider>());
		//physicsSig.set(world.getComponentType<RigidBody2D>());
		world.setSystemSignature<PhysicsSystem>(physicsSig);

		Signature animationSig;
		animationSig.set(world.getComponentType<EngineTransform>());
		animationSig.set(world.getComponentType<Animator2D>());
		world.setSystemSignature<AnimationSystem>(animationSig);


		// Load entities from file
		/*FILE* f = fopen("sd:/samplescene.bin", "rb");
		if (f) {
			fclose(f);
			world.load("sd:/samplescene.bin");
		}
		else {
			// Create new/default entities here
			Entity e = world.createEntity();
			world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 640 / 2, 480 / 2,0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
			PrimitiveRenderable2D dot;
			dot.shape = RenderShape2D::Circle;
			dot.circle.radius = 25.0f;
			world.addComponent(e, dot);
			return;
		}*/

		// Create new/default entities here
		e = world.createEntity();
		world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 0, 0, 0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
		Renderable model;
		model.shape = RenderShape::ModelWires;
		model.color = WHITE;
		model.model.modelId = teapotId;
		model.model.scale = 1.0f;
		world.addComponent(e, model);
		BoxCollider collider;
		collider.entityId = e;
		collider.bounds = world.getModelBoundingBox(teapotId);
		collider.UpdateFromBounds();
		world.addComponent(e, collider);


		other = world.createEntity();
		world.addComponent<EngineTransform>(other, EngineTransform(Vector3{ 0, 5, 0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
		//Renderable modelother;
		//model.shape = RenderShape::ModelWires;
		//model.color = WHITE;
		//model.model.modelId = teapotId;
		//model.model.scale = 1.0f;
		world.addComponent(other, model);
		BoxCollider colliderother;
		colliderother.entityId = other;
		colliderother.bounds = world.getModelBoundingBox(teapotId);
		colliderother.UpdateFromBounds();
		world.addComponent(other, colliderother);
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		u32 expType;
		s32 result = WPAD_Probe(WPAD_BALANCE_BOARD, &expType);
		board = (result == WPAD_ERR_NONE);

		UpdateCamera(&camera, CAMERA_ORBITAL);

		struct expansion_t exp;
		WPAD_Expansion(WPAD_BALANCE_BOARD, &exp);
		wb = &exp.wb;
		total = wb->tl + wb->tr + wb->bl + wb->br;
		x = ((wb->tr + wb->br) / total) * 2 - 1;
		y = ((wb->tl + wb->tr) / total) * 2 - 1;

		screenX = ((x + 1.0f) * 0.5f) * 640.0f;
		screenY = ((1.0f - y) * 0.5f) * 480.0f;

		// update physics system
		physics->update(dt);
		physics->updateCollisions(dt, true);

		// update animation system
		animation->update(dt);

		EngineTransform& transform = world.getComponent<EngineTransform>(other);
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP) transform.pos.z += 1.0f * dt;;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN) transform.pos.z -= 1.0f * dt;;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_LEFT) transform.pos.x -= 1.0f * dt;;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_RIGHT) transform.pos.x += 1.0f * dt;;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_A) transform.pos.y += 1.0f * dt;;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_B) transform.pos.y -= 1.0f * dt;;

		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_1) camera.position.z += 1.0f * dt;
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_2) camera.position.z -= 1.0f * dt;

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS) {
			Entity newe = world.createEntity();
			world.addComponent<EngineTransform>(newe, EngineTransform(Vector3{ GetRandomValue(-10, 10), GetRandomValue(-10, 10), GetRandomValue(-10, 10) }, Vector3{0,0,0}, Vector3{1,1,1}));
			Renderable model;
			model.shape = RenderShape::ModelWires;
			model.color = WHITE;
			model.model.modelId = teapotId;
			model.model.scale = 1.0f;
			world.addComponent(newe, model);
			BoxCollider colliderother;
			colliderother.entityId = newe;
			colliderother.bounds = world.getModelBoundingBox(teapotId);
			colliderother.UpdateFromBounds();
			world.addComponent(newe, colliderother);
		}
	}

	void render(float dt) override {
		BeginDrawing();
		rendersys->SetDrawMode3D(&camera);
		ClearBackground(BLACK);
		glClear(GL_DEPTH_BUFFER_BIT);
		rendersys->update(dt);

		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_MINUS) {
			physics->rayTest(PhysicsRay(Vector3{ world.getComponent<EngineTransform>(other).pos.x + world.getComponent<BoxCollider>(other).halfExtents.x + 1.0f,
												 world.getComponent<EngineTransform>(other).pos.y,
												 world.getComponent<EngineTransform>(other).pos.z },
				Vector3{ 1, 0, 0 }));
		}

		physics->drawDebug();

		if (board) {
			//DrawText("Board Connected", 10, 30, 20, WHITE);
			//DrawText(TextFormat("X: %f, Y: %f)", x, y), 10, 50, 20, WHITE);
			//DrawText(TextFormat("ScreenX: %f, ScreenY: %f)", screenX, screenY), 10, 70, 20, WHITE);
			//DrawText(TextFormat("Raw weight: TL:%d  TR:%d", wb->tl, wb->tr), 10, 70, 20, WHITE);
			//DrawText(TextFormat("Raw weight: BL:%d  BR:%d", wb->bl, wb->br), 10, 90, 20, WHITE);
			//DrawText("Board Connected", 10, 30, 20, WHITE);

		}
		else {
			//DrawText("Board Disconnected", 10, 30, 20, WHITE);
		}

		rendersys->SetDrawMode2D();
		DrawText(TextFormat("%f FPS", 1/dt), 10, 10, 20, RED);
		EndDrawing();
	}

	void shutdown() override {
		// save current scene state
		//world.save("sd:/samplescene.bin");
		// unload scene resources
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};