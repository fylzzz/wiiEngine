#include "PhysicsSystem.h"
#include <math.h>


class OctTree {
	private:
		int mLevel;
		BoundingBox mBounds;
		std::vector<BoxCollider> mObjects;
		std::array<std::unique_ptr<OctTree>, 8> mNodes;

		const size_t MAX_OBJECTS = 4;
		const int MAX_LEVELS = 5;

		void split() {
			Vector3 min = mBounds.min;
			Vector3 max = mBounds.max;
			Vector3 mid = {
				(mBounds.min.x + mBounds.max.x) * 0.5f,
				(mBounds.min.y + mBounds.max.y) * 0.5f,
				(mBounds.min.z + mBounds.max.z) * 0.5f
			};

			mNodes[0] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { mid.x, min.y, min.z }, { max.x, mid.y, mid.z} });
			mNodes[1] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { min.x, min.y, min.z }, { mid.x, mid.y, mid.z} });
			mNodes[2] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { min.x, mid.y, min.z }, { mid.x, max.y, mid.z} });
			mNodes[3] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { mid.x, mid.y, min.z }, { max.x, max.y, mid.z} });
			mNodes[4] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { mid.x, min.y, mid.z }, { max.x, mid.y, max.z} });
			mNodes[5] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { min.x, min.y, mid.z }, { mid.x, mid.y, max.z} });
			mNodes[6] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { min.x, mid.y, mid.z }, { mid.x, max.y, max.z} });
			mNodes[7] = std::make_unique<OctTree>(mLevel + 1, BoundingBox{ { mid.x, mid.y, mid.z }, { max.x, max.y, max.z} });
		}

		int getIndex(const BoundingBox& bounds) const {
			Vector3 min = bounds.min;
			Vector3 max = bounds.max;
			Vector3 mid = {
				(mBounds.min.x + mBounds.max.x) * 0.5f,
				(mBounds.min.y + mBounds.max.y) * 0.5f,
				(mBounds.min.z + mBounds.max.z) * 0.5f
			};

			bool fitsLeft = max.x < mid.x;
			bool fitsRight = min.x > mid.x;
			bool fitsBottom = max.y < mid.y;
			bool fitsTop = min.y > mid.y;
			bool fitsNear = max.z < mid.z;
			bool fitsFar = min.z > mid.z;

			if (fitsNear) {
				if (fitsRight && fitsBottom) return 0;
				if (fitsLeft && fitsBottom) return 1;
				if (fitsLeft && fitsTop) return 2;
				if (fitsRight && fitsTop) return 3;
			}
			else if (fitsFar) {
				if (fitsRight && fitsBottom) return 4;
				if (fitsLeft && fitsBottom) return 5;
				if (fitsLeft && fitsTop) return 6;
				if (fitsRight && fitsTop) return 7;
			}
			return -1;
		}
	public:
		OctTree(int level, const BoundingBox& bounds) : mLevel(level), mBounds(bounds) {}

		void clear() {
			mObjects.clear();
			for (auto& node : mNodes) {
				if (node) {
					node->clear();
					node = nullptr;
				}
			}
		}

		void insert(const BoxCollider& col) {
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
					} else {
						++it;
					}
				}
			}
		}

		void retrieve(std::vector<BoxCollider>& returncol, const BoundingBox& col) const {
			int index = getIndex(col);
			if (index != -1 && mNodes[0] != nullptr) {
				mNodes[index]->retrieve(returncol, col);
			}
			else if (mNodes[0] != nullptr) {
				for (const auto& node : mNodes) {
					node->retrieve(returncol, col);
				}
			}
			returncol.insert(returncol.end(), mObjects.begin(), mObjects.end());
		}
};


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
			float vertMidpoint = mBounds.x + (mBounds.width / 2.0f);
			float horMidpoint = mBounds.y + (mBounds.height / 2.0f);

			bool topQuad = (rect.y < horMidpoint && rect.y + rect.height < horMidpoint);
			bool bottomQuad = (rect.y > horMidpoint);

			if (rect.x < vertMidpoint && rect.x + rect.width < vertMidpoint) {
				if (topQuad) return 1;
				if (bottomQuad) return 2;
			}
			else if (rect.x > vertMidpoint) {
				if (topQuad) return 0;
				if (bottomQuad) return 3;
			}
			return -1;
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

static bool aabbOverlap(const Rectangle& a, const Rectangle& b) {
	return !(a.x + a.width < b.x ||
		b.x + b.width < a.x ||
		a.y + a.height < b.y ||
		b.y + b.height < a.y);
}

static bool aabbOverlap3D(const BoundingBox& a, const BoundingBox& b) {
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}


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
		if (world->hasComponent<Collider2D>(e)) {
			auto& col = world->getComponent<Collider2D>(e);
			DrawRectangleLines(col.bounds.x, col.bounds.y, col.bounds.width, col.bounds.height, GREEN);
		}
		if (world->hasComponent<BoxCollider>(e)) {
			auto& col3D = world->getComponent<BoxCollider>(e);
			DrawBoundingBox(col3D.bounds, col3D.isColliding ? RED : GREEN);
		}
	}
}

void PhysicsSystem::updateCollisions(float dt, bool drawBounds) {
	QuadTree qt(0, Rectangle{ 0, 0, 640, 480 });
	OctTree ot(0, BoundingBox{ Vector3{ -10, -10, -10 }, Vector3{ 10, 10, 10 } });

	for (Entity e : mEntities) {
		if (!world->hasComponent<EngineTransform>(e)) continue;
		auto& trans = world->getComponent<EngineTransform>(e);

		if (world->hasComponent<Collider2D>(e)) {
			auto& col = world->getComponent<Collider2D>(e);
			col.bounds.x = trans.pos.x + col.offset.x;
			col.bounds.y = trans.pos.y + col.offset.y;
			col.entityId = e;
			qt.insert(col);
		}

		if (world->hasComponent<BoxCollider>(e)) {
			auto& col3D = world->getComponent<BoxCollider>(e);
			col3D.bounds.min = { trans.pos.x + col3D.center.x - col3D.halfExtents.x,
								  trans.pos.y + col3D.center.y - col3D.halfExtents.y,
								  trans.pos.z + col3D.center.z - col3D.halfExtents.z };
			col3D.bounds.max = { trans.pos.x + col3D.center.x + col3D.halfExtents.x,
								  trans.pos.y + col3D.center.y + col3D.halfExtents.y,
								  trans.pos.z + col3D.center.z + col3D.halfExtents.z };
			col3D.entityId = e;
			ot.insert(col3D);
		}
	}

	for (Entity e : mEntities) {
		if (world->hasComponent<Collider2D>(e)) {
			auto& col = world->getComponent<Collider2D>(e);

			std::vector<Collider2D> candidates;
			qt.retrieve(candidates, col.bounds);

			for (const auto& other : candidates) {
				if (other.entityId == col.entityId) continue;

				bool hit = aabbOverlap(col.bounds, other.bounds);
				if (hit) {
					if (!world->hasComponent<RigidBody2D>(e)) continue;
					auto& rb = world->getComponent<RigidBody2D>(e);
					if (rb.type == RbType::Dynamic) {
						rb.velocity.x = -rb.velocity.x;
						rb.velocity.y = -rb.velocity.y;
					}
				}
			}
		}

		if (world->hasComponent<BoxCollider>(e)) {
			auto& col3D = world->getComponent<BoxCollider>(e);

			std::vector<BoxCollider> candidates3D;
			ot.retrieve(candidates3D, col3D.bounds);

			col3D.isColliding = false;
			for (const auto& other3D : candidates3D) {
				if (other3D.entityId == col3D.entityId) continue;

				bool hit = aabbOverlap3D(col3D.bounds, other3D.bounds);
				if (hit) {
					col3D.isColliding = true;
					break;
				}
			}
		}
	}
}

bool PhysicsSystem::isColliding(Entity a, Entity b) const {
	if (a == b) return false;

	if (!world->hasComponent<Collider2D>(a) || !world->hasComponent<Collider2D>(b)) return false;
	if (!world->hasComponent<EngineTransform>(a) || !world->hasComponent<EngineTransform>(b)) return false;

	auto& colA = world->getComponent<Collider2D>(a);
	auto& colB = world->getComponent<Collider2D>(b);
	//auto& transA = world->getComponent<EngineTransform>(a);
	//auto& transB = world->getComponent<EngineTransform>(b);

	Rectangle boundsA{ colA.bounds.x, colA.bounds.y, colA.bounds.width, colA.bounds.height };
	Rectangle boundsB{ colB.bounds.x, colB.bounds.y, colB.bounds.width, colB.bounds.height };

	return aabbOverlap(boundsA, boundsB);
}