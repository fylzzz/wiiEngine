#pragma once

#include "Scene.h"
#include <memory>


constexpr uint8_t MAX_SCENES = 16;

class SceneManager {
	public:
		template<typename T>
		void registerScene(uint8_t id) {
			mScenes[id] = std::make_shared<T>();
		}

		void switchTo(uint8_t id) {
			if (mCurrent && mScenes[mCurrentId]) {
				mScenes[mCurrentId]->shutdown();
			}

			mCurrentId = id;
			mCurrent = mScenes[id].get();
			mCurrent->sceneManager = this;

			if (mCurrent) {
				mCurrent->world.init();
				mCurrent->init();
			}
		}

		void update(float dt) {
			if (mCurrent) {
				mCurrent->update(dt);
			}
		}
		void render(float dt) {
			if (mCurrent) {
				mCurrent->render(dt);
			}
		}

		Scene* current() {
			return mCurrent;
		}

	private:
		std::array<std::shared_ptr<Scene>, MAX_SCENES> mScenes{};
		Scene* mCurrent = nullptr;
		uint8_t mCurrentId = 0;
};