#pragma once

#include "World.h"
//#define RAYGUI_IMPLEMENTATION
//#include "raygui.h"


class SceneManager;

class Scene {
	public:
		World world;
		SceneManager* sceneManager = nullptr;

		virtual ~Scene() = default;

		virtual void init() = 0;
		virtual void update(float dt, WPADData* data) = 0;
		virtual void render(float dt) = 0;
		virtual void shutdown() = 0;
};