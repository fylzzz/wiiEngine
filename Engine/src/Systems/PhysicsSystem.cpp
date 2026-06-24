#include "PhysicsSystem.h"
#include <math.h>


class QuadTree {
	private:
		int mLevel;
		Rectangle mBounds;
		std::vector<Collider2D> mObjects;
		std::array<std::unique_ptr<QuadTree>, 4> mNodes;

		const size_t MAX_OBJECTS = 4;
		const int MAX_LEVELS = 5;

		void split() {
			float subwidth = mBounds.width / 2.0f;
			float subheight = mBounds.height / 2.0f;
			float x = mBounds.x;
			float y = mBounds.y;

			mNodes[0] = std::make_unique<QuadTree>(mLevel + 1, Rectangle{ x + subwidth, y, subwidth, subheight });
			mNodes[1] = std::make_unique<QuadTree>(mLevel + 1, Rectangle{ x, y, subwidth, subheight });
			mNodes[2] = std::make_unique<QuadTree>(mLevel + 1, Rectangle{ x, y + subheight, subwidth, subheight });
			mNodes[3] = std::make_unique<QuadTree>(mLevel + 1, Rectangle{ x + subwidth, y + subheight, subwidth, subheight });
		}

		int getIndex(const Rectangle& rect) const {
			int index = -1;
			float vertMidpoint = mBounds.x + (mBounds.width / 2.0f);
			float horMidpoint = mBounds.y + (mBounds.height / 2.0f);

			bool topQuad = (rect.y < horMidpoint && rect.y + rect.height < horMidpoint);
			bool bottomQuad = (rect.y > horMidpoint);

			if (rect.x < vertMidpoint && rect.x + rect.width < vertMidpoint) {
				if (topQuad) {
					index = 1;
				}
				else if (bottomQuad) {
					index = 2;
				}
			}
			else if (rect.x > vertMidpoint) {
				if (topQuad) {
					index = 0;
				}
				else if (bottomQuad) {
					index = 3;
				}
			}
			return index;
		}

	public:
		QuadTree(int level, const Rectangle& bounds) : mLevel(level), mBounds(bounds) {}

		void clear() {
			mObjects.clear();
			for (auto& node : mNodes) {
				if (node) {
					node->clear();
					node = nullptr;
				}
			}
		}

		void insert(const Collider2D& col) {
			if (mNodes[0] != nullptr) {
				int index = getIndex(col.bounds);
				if (index != -1) {
					mNodes[index]->insert(col);
					return;
				}
			}

			mObjects.push_back(col);

			if (mObjects.size() > MAX_OBJECTS && mLevel < MAX_LEVELS) {
				if (mNodes[0] == nullptr) {
					split();
				}
				auto it = mObjects.begin();
				while (it != mObjects.end()) {
					int index = getIndex(it->bounds);
					if (index != -1) {
						mNodes[index]->insert(*it);
						it = mObjects.erase(it);
					}
					else {
						++it;
					}
				}
			}
		}

		void retrieve(std::vector<Collider2D>& returncol, const Rectangle& rect) const {
			int index = getIndex(rect);
			if (index != -1 && mNodes[0] != nullptr) {
				mNodes[index]->retrieve(returncol, rect);
			}
			else if (mNodes[0] != nullptr) {
				for (const auto& node : mNodes) {
					node->retrieve(returncol, rect);
				}
			}
			returncol.insert(returncol.end(), mObjects.begin(), mObjects.end());
		}
};

void PhysicsSystem::update(float dt) {
	for (Entity e : mEntities) {
		if (!world->hasComponent<RigidBody2D>(e)) continue;
		auto& trans = world->getComponent<EngineTransform>(e);
		auto rb2d = world->getComponent<RigidBody2D>(e);

		trans.pos.x += rb2d.velocity.x;
		trans.pos.y += rb2d.velocity.y;
	}
}
void PhysicsSystem::drawDebug() {
	for (Entity e : mEntities) {
		if (!world->hasComponent<Collider2D>(e)) continue;
		auto& col = world->getComponent<Collider2D>(e);
		auto& trans = world->getComponent<EngineTransform>(e);

		DrawRectangleLines(trans.pos.x, trans.pos.y, col.bounds.width, col.bounds.height, GREEN);
	}
}

void PhysicsSystem::updateCollisions(float dt, bool drawBounds) {
	QuadTree qt(0, Rectangle{ 0, 0, 640, 480 });

	for (Entity e : mEntities) {
		if (!world->hasComponent<Collider2D>(e)) continue;
		auto& col = world->getComponent<Collider2D>(e);
		auto& trans = world->getComponent<EngineTransform>(e);

		col.bounds.x = trans.pos.x;
		col.bounds.y = trans.pos.y;
		col.entityId = e;
		if (drawBounds) {
			DrawRectangleLines(trans.pos.x, trans.pos.y, col.bounds.width, col.bounds.height, GREEN);
		}
		qt.insert(col);
	}

	for (Entity e : mEntities) {
		if (!world->hasComponent<Collider2D>(e)) continue;
		auto& col = world->getComponent<Collider2D>(e);

		std::vector<Collider2D> candidates;
		qt.retrieve(candidates, col.bounds);

		for (const auto& other : candidates) {
			if (other.entityId == col.entityId) continue;

			bool hit = !(col.bounds.x + col.bounds.width < other.bounds.x ||
				other.bounds.x + other.bounds.width < col.bounds.x ||
				col.bounds.y + col.bounds.height < other.bounds.y ||
				other.bounds.y + other.bounds.height < col.bounds.y);

			if (hit) {
				if (!world->hasComponent<RigidBody2D>(e)) continue;
				auto& rb = world->getComponent<RigidBody2D>(e);
				rb.velocity.x = -rb.velocity.x;
				rb.velocity.y = -rb.velocity.y;
			}
		}  
	}
}