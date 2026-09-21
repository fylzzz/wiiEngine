#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <wiiuse/wpad.h>
#include <math.h>
#include <algorithm>


void SetDrawMode3D(Camera3D* camera) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = 640.0f / 480.0f;
	float near = 0.05f, far = 1000.0f;
	float t = near * tanf(camera->fovy * 0.5f * 3.14159f / 180.0f);
	glFrustum(-t * aspect, t * aspect, -t, t, near, far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Manual lookAt
	Vector3 eye = camera->position;
	Vector3 center = camera->target;
	Vector3 up = camera->up;

	// forward = normalize(center - eye)
	float fx = center.x - eye.x, fy = center.y - eye.y, fz = center.z - eye.z;
	float fl = sqrtf(fx * fx + fy * fy + fz * fz);
	fx /= fl; fy /= fl; fz /= fl;

	// right = normalize(forward x up)
	float rx = fy * up.z - fz * up.y;
	float ry = fz * up.x - fx * up.z;
	float rz = fx * up.y - fy * up.x;
	float rl = sqrtf(rx * rx + ry * ry + rz * rz);
	rx /= rl; ry /= rl; rz /= rl;

	// up = right x forward
	float ux = ry * fz - rz * fy;
	float uy = rz * fx - rx * fz;
	float uz = rx * fy - ry * fx;

	float m[16] = {
		rx, ux, -fx, 0,
		ry, uy, -fy, 0,
		rz, uz, -fz, 0,
		-(rx * eye.x + ry * eye.y + rz * eye.z),
		-(ux * eye.x + uy * eye.y + uz * eye.z),
		(fx * eye.x + fy * eye.y + fz * eye.z), 1
	};
	glMultMatrixf(m);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

class WeightTest : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	ResourceId playerId;
	ResourceId enemyId;
	std::vector<Entity> enemy;
	enum state { MENU, PLAY, END };
	state gameState;

	float spawnTimer = 0.0f;
	float distance = 0.0f;
	float score = 0.0f;


	bool board;
	struct wii_board_t* wb;

	float calTL = 0, calTR = 0, calBL = 0, calBR = 0;
	bool calibrated = false;
	int calFrames = 0;
	static const int CAL_SAMPLE_COUNT = 60;

	float total, x, y, screenX, screenY;

	Entity e;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		playerId = world.loadModel("sd:/box.obj");
		enemyId = world.loadModel("sd:/sphere.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		// Setup camera
		camera.position = Vector3{ 0.0f, 10.0f, 10.0f };
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
		world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
		Renderable mesh;
		mesh.shape = RenderShape::ModelWires;
		mesh.color = WHITE;
		mesh.model.modelId = playerId;
		mesh.model.scale = 1.0f;
		world.addComponent(e, mesh);
		BoxCollider col;
		col.entityId = e;
		col.bounds = world.getModelBoundingBox(playerId);
		col.UpdateFromBounds();
		world.addComponent(e, col);

		gameState = PLAY;
	}

	void update(float dt, WPADData* data) override {

		UpdateCamera(&camera, CAMERA_PERSPECTIVE);

		EngineTransform& transform = world.getComponent<EngineTransform>(e);

		// update inputs, entities, camera etc. here
		distance += dt;
		spawnTimer += dt;
		if (spawnTimer >= 1.5f) {
			Entity newenemy = world.createEntity();
			world.addComponent<EngineTransform>(newenemy, EngineTransform(Vector3{ GetRandomValue(7,-7), GetRandomValue(7,-7), -20}, Vector3{0,0,0}, Vector3{1,1,1}));
			Renderable enemyMesh;
			enemyMesh.shape = RenderShape::ModelWires;
			enemyMesh.color = WHITE;
			enemyMesh.model.modelId = enemyId;
			enemyMesh.model.scale = 1.0f;
			world.addComponent(newenemy, enemyMesh);
			BoxCollider enemyCol;
			enemyCol.entityId = newenemy;
			enemyCol.bounds = world.getModelBoundingBox(enemyId);
			enemyCol.UpdateFromBounds();
			world.addComponent(newenemy, enemyCol);

			enemy.push_back(newenemy);

			spawnTimer = 0.0f;
		}

		for (auto it = enemy.begin(); it != enemy.end(); ) {
			EngineTransform& enemyTrans = world.getComponent<EngineTransform>(*it);
			enemyTrans.pos.z += 5.0f * dt;

			if (enemyTrans.pos.z > 30.0f) {
				world.destroyEntity(*it);
				it = enemy.erase(it);
			}
			else {
				++it;
			}
		}

		u32 expType;
		s32 result = WPAD_Probe(WPAD_BALANCE_BOARD, &expType);
		board = (result == WPAD_ERR_NONE);

		struct expansion_t exp;
		WPAD_Expansion(WPAD_BALANCE_BOARD, &exp);
		wb = &exp.wb;
		if (!calibrated) {
			calTL += wb->tl;
			calTR += wb->tr;
			calBL += wb->bl;
			calBR += wb->br;
			calFrames++;
			if (calFrames >= CAL_SAMPLE_COUNT) {
				calTL /= calFrames;
				calTR /= calFrames;
				calBL /= calFrames;
				calBR /= calFrames;
				calibrated = true;
			}
		}
		else {
			float tl = fmaxf(0.0f, wb->tl - calTL);
			float tr = fmaxf(0.0f, wb->tr - calTR);
			float bl = fmaxf(0.0f, wb->bl - calBL);
			float br = fmaxf(0.0f, wb->br - calBR);

			total = tl + tr + bl + br;
			if (total > 0.5f) {
				x = ((tr + br) / total) * 2 - 1;
				y = ((tl + tr) / total) * 2 - 1;
				x = fmaxf(-1.0f, fminf(1.0f, x));
				y = fmaxf(-1.0f, fminf(1.0f, y));
			}
			else {
				x = 0.0f;
				y = 0.0f;
			}
		}

		float worldX = x * 10.0f;
		float worldY = y * 10.0f;

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
			calibrated = false;
			calFrames = 0;
			calTL = calTR = calBL = calBR = 0;
		}

		// update physics system
		physics->update(dt);
		physics->updateCollisions(dt, false);

		// update animation system
		animation->update(dt);

		transform.pos.x = worldX;
		transform.pos.y = worldY;
	}

	void render(float dt) override {
		BeginDrawing();
		ClearBackground(BLACK);
		glClear(GL_DEPTH_BUFFER_BIT);
		rendersys->SetDrawMode3D(&camera);
		rendersys->update(dt);

		physics->drawDebug();

		switch (gameState) {
		case MENU:
			break;
		case PLAY:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) {
				Entity* hitEntity = physics->rayTest(PhysicsRay(Vector3{ world.getComponent<EngineTransform>(e).pos.x,
													 world.getComponent<EngineTransform>(e).pos.y,
													 world.getComponent<EngineTransform>(e).pos.z + world.getComponent<BoxCollider>(e).halfExtents.z },
					Vector3{ 0, 0, -1 }));

				if (hitEntity) {
					Entity hitID = *hitEntity;
					if (hitID != e) {
						world.destroyEntity(hitID);
						enemy.erase(std::remove(enemy.begin(), enemy.end(), hitID), enemy.end());
					}
				}
			}
			rendersys->SetDrawMode2D();
			DrawText(TextFormat("Distance: %f", distance), 10, 460, 20, WHITE);
			DrawText(TextFormat("Score: %f", score), 10, 440, 20, WHITE);
			break;
		case END:
			break;
		}

		if (board) {
			DrawText("Board Connected", 10, 30, 20, WHITE);
			DrawText(TextFormat("X: %f, Y: %f)", x, y), 10, 50, 20, WHITE);
			DrawText(TextFormat("ScreenX: %f, ScreenY: %f)", screenX, screenY), 10, 70, 20, WHITE);
		}
		else {
			DrawText("Board Disconnected", 10, 30, 20, WHITE);
		}


		DrawFPS(10, 10);
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