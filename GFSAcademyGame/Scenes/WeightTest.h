#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "MusicStream.h"

#include <math.h>
#include <algorithm>


class WeightTest : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	MusicStream music;
	Camera3D camera = {};

	ResourceId playerId;
	ResourceId enemyId;

	AudioId test;

	std::vector<Entity> enemy;
	enum state { MENU, PLAY, END };
	state gameState;

	float spawnTimer = 0.0f;
	float distance = 0.0f;
	float score = 0.0f;
	int lives = 3;

	float rumbleTimer = 0.0f;
	bool takeDamage = false;

	// last shot, kept so render() can draw it
	static constexpr float SHOT_DISPLAY_TIME = 0.1f;
	float shotTimer = 0.0f;
	Vector3 shotStart = {}, shotEnd = {};

	float calTL = 0, calTR = 0, calBL = 0, calBR = 0;
	bool calibrated = false;
	int calFrames = 0;
	static const int CAL_SAMPLE_COUNT = 60;

	float total, x, y, screenX, screenY;

	Entity e;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		playerId = world.loadModel("sd:/ship.obj");
		enemyId = world.loadModel("sd:/sphere.obj");
		test = world.loadAudio("test", "sd:/test.wav");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		// Setup camera
		camera.position = Vector3{ 0.0f, 0.0f, 10.0f };
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
		world.registerComponent<MeshCollider>();
		world.registerComponentSerializer<MeshCollider>();
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
		physicsSig.set(world.getComponentType<MeshCollider>());
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
		gameState = MENU;

		music.loadTrack("sd:/maintheme_final_mix2.wav");
		music.playTrack(255, 255);
	}

	void update(float dt, InputState& input) override {

		switch (gameState) {
			case MENU:
				if (input.wasPressed(BTN_A)) {
					e = world.createEntity();
					world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 0,0,5.0f }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
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

					MeshCollider meshcol;
					meshcol.entityId = e;
					meshcol.localVertices = meshcol.GetModelVertices(world.getModel(playerId));
					world.addComponent(e, meshcol);

					distance = 0.0f;
					score = 0.0f;
					lives = 3;

					gameState = PLAY;
				}
				break;
			case PLAY: {
				UpdateCamera(&camera, CAMERA_PERSPECTIVE);
				EngineTransform& transform = world.getComponent<EngineTransform>(e);

				const BalanceBoardState& wb = input.board;

				if (!calibrated) {
					calTL += wb.tl;
					calTR += wb.tr;
					calBL += wb.bl;
					calBR += wb.br;
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
					float tl = fmaxf(0.0f, wb.tl - calTL);
					float tr = fmaxf(0.0f, wb.tr - calTR);
					float bl = fmaxf(0.0f, wb.bl - calBL);
					float br = fmaxf(0.0f, wb.br - calBR);

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

				distance += dt * 5;
				spawnTimer += dt;
				if (spawnTimer >= 1.5f) {
					Entity newenemy = world.createEntity();
					world.addComponent<EngineTransform>(newenemy, EngineTransform(Vector3{ GetRandomValue(7,-7), GetRandomValue(7,-7), -20 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));

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

					MeshCollider enemyMeshCol;
					enemyMeshCol.entityId = newenemy;
					enemyMeshCol.localVertices = enemyMeshCol.GetModelVertices(world.getModel(enemyId));
					world.addComponent(newenemy, enemyMeshCol);

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

				transform.pos.x = worldX;
				transform.pos.y = worldY;

				if (shotTimer > 0.0f) shotTimer -= dt;

				if (input.wasPressed(BTN_B)) {
					world.playAudio(test, 255, 255);
					const EngineTransform& pt = world.getComponent<EngineTransform>(e);
					Vector3 origin = { pt.pos.x, pt.pos.y, pt.pos.z - world.getComponent<BoxCollider>(e).halfExtents.z };
					Vector3 dir = { 0, 0, -1 };

					shotStart = origin;
					shotEnd = Vector3Add(origin, Vector3Scale(dir, 100.0f));
					shotTimer = SHOT_DISPLAY_TIME;

					Entity* hitEntity = physics->rayTest(PhysicsRay(origin, dir));

					if (hitEntity) {
						Entity hitID = *hitEntity;
						if (hitID != e) {
							world.destroyEntity(hitID);
							enemy.erase(std::remove(enemy.begin(), enemy.end(), hitID), enemy.end());
							score += 100;
						}
					}
				}

				physics->update(dt);
				physics->updateCollisions(dt, false);

				if (world.getComponent<MeshCollider>(e).isColliding) {
					input.rumble = true;
					world.playAudio(test, 150, 150);
					takeDamage = true;
					rumbleTimer = 0.0f;
					lives--;
					Entity hitEnemy = physics->getMeshCollision(e);
					world.destroyEntity(hitEnemy);
					enemy.erase(std::remove(enemy.begin(), enemy.end(), hitEnemy), enemy.end());
				}
				if (lives == 0) {
					world.destroyAllEntities();
					enemy.clear();
					gameState = END;
				}

				if (takeDamage) {
					rumbleTimer += dt;
					if (rumbleTimer >= 0.5f) {
						takeDamage = false;
						input.rumble = false;
					}
				}


				break;
			}
			case END:
				if (input.wasPressed(BTN_A)) {
					gameState = MENU;
				}
				break;
		}

		// update inputs, entities, camera etc. here
		if (input.wasPressed(BTN_1)) {
			calibrated = false;
			calFrames = 0;
			calTL = calTR = calBL = calBR = 0;
		}

		// update physics system

		// update animation system
		animation->update(dt);

		// update music stream
		music.update();
	}

	void render(float dt) override {

		switch (gameState) {
		case MENU:
			rendersys->SetDrawMode2D();

			DrawText("Press A to Start", 10, 10, 20, WHITE);
			break;
		case PLAY:
			rendersys->SetDrawMode3D(&camera);

			rendersys->update(dt);
			if (shotTimer > 0.0f)
				PhysicsRay::DrawRay(shotStart, shotEnd, 1.0f, WHITE);
			//physics->drawDebug();

			rendersys->SetDrawMode2D();

			DrawText(TextFormat("Distance: %.0f", distance), 10, 10, 20, WHITE);
			DrawText(TextFormat("Score: %.0f", score), 10, 30, 20, WHITE);
			DrawText(TextFormat("Lives: %i", lives), 10, 50, 20, WHITE);

			break;
		case END:
			rendersys->SetDrawMode2D();

			DrawText("Game Over", 10, 10, 20, WHITE);
			DrawText(TextFormat("Distance: %.0f", distance), 10, 30, 20, WHITE);
			DrawText(TextFormat("Score: %.0f", score), 10, 50, 20, WHITE);
			DrawText("Press A to return to title", 10, 90, 20, WHITE);
			break;
		}

#ifdef GEKKO
		s32 v = music.voice();
		DrawText(TextFormat("v:%d st:%d rdy:%d upd:%d fills:%d prime:%d add:%d",
			v, v >= 0 ? ASND_StatusVoice(v) : -9, v >= 0 ? ASND_TestVoiceBufferReady(v) : -9,
			music.dbgUpdates, music.dbgFills, music.dbgPrimeRet, music.dbgAddRet),
			10, 450, 16, YELLOW);
#endif

		//DrawFPS(10, 10);
	}

	void shutdown() override {
		// save current scene state
		//world.save("sd:/samplescene.bin");
		// unload scene resources
		music.unloadTrack();
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};