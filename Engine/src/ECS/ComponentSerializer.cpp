#include "Components.h"


static void writeRect(Writer& w, Rectangle r) { w.f32(r.x); w.f32(r.y); w.f32(r.width); w.f32(r.height); }
static Rectangle readRect(Reader& r) {
	Rectangle o;
	o.x = r.f32(); o.y = r.f32(); o.width = r.f32(); o.height = r.f32();
	return o;
}

// ---- EngineTransform ----
void serialize(Writer& w, const EngineTransform& c) { w.vec3(c.pos); w.vec3(c.rot); w.vec3(c.sca); }
void deserialize(Reader& r, EngineTransform& c) { c.pos = r.vec3(); c.rot = r.vec3(); c.sca = r.vec3(); }

// ---- PrimitiveRenderable2D ----
void serialize(Writer& w, const PrimitiveRenderable2D& c) {
	w.u8(uint8_t(c.shape));
	w.color(c.color);
	switch (c.shape) {
	case RenderShape2D::Rectangle: w.i32(c.rectangle.width); w.i32(c.rectangle.height); break;
	case RenderShape2D::Circle:    w.f32(c.circle.radius); break;
	case RenderShape2D::Triangle:  w.vec2(c.triangle.v1); w.vec2(c.triangle.v2); w.vec2(c.triangle.v3); break;
	case RenderShape2D::Ellipse:   w.f32(c.ellipse.radiusH); w.f32(c.ellipse.radiusV); break;
	}
}
void deserialize(Reader& r, PrimitiveRenderable2D& c) {
	c.shape = RenderShape2D(r.u8());
	c.color = r.color();
	switch (c.shape) {
	case RenderShape2D::Rectangle: c.rectangle.width = r.i32(); c.rectangle.height = r.i32(); break;
	case RenderShape2D::Circle:    c.circle.radius = r.f32(); break;
	case RenderShape2D::Triangle:  c.triangle.v1 = r.vec2(); c.triangle.v2 = r.vec2(); c.triangle.v3 = r.vec2(); break;
	case RenderShape2D::Ellipse:   c.ellipse.radiusH = r.f32(); c.ellipse.radiusV = r.f32(); break;
	}
}

// ---- Renderable2D ----  (texture is a GPU handle: rebuilt from spriteId in step 3)
void serialize(Writer& w, const Renderable2D& c) { w.u16(c.spriteId); w.color(c.color); }
void deserialize(Reader& r, Renderable2D& c) { c.spriteId = r.u16(); c.color = r.color(); }

// ---- Animator2D ----  (playback state is runtime, not saved)
void serialize(Writer& w, const Animator2D& c) {
	w.str(c.currentAnim);
	w.u32(uint32_t(c.animClips.size()));
	for (const auto& [name, id] : c.animClips) { w.str(name); w.u16(id); }
	w.color(c.color);
}
void deserialize(Reader& r, Animator2D& c) {
	c.currentAnim = r.str();
	uint32_t n = r.u32();
	if (n > 256) n = 0;   // corrupt-count guard; Reader's error flag catches the rest
	for (uint32_t i = 0; i < n && r.ok(); ++i) {
		std::string name = r.str();
		c.animClips[name] = r.u16();
	}
	c.color = r.color();
}

// ---- Renderable ----
void serialize(Writer& w, const Renderable& c) {
	w.u8(uint8_t(c.shape));
	w.color(c.color);
	switch (c.shape) {
	case RenderShape::Cube:     w.f32(c.cube.width); w.f32(c.cube.height); w.f32(c.cube.length); break;
	case RenderShape::Sphere:   w.f32(c.sphere.radius); break;
	case RenderShape::Cylinder: w.f32(c.cylinder.topRadius); w.f32(c.cylinder.bottomRadius);
		w.f32(c.cylinder.height); w.i32(c.cylinder.slices); break;
	case RenderShape::Capsule:  w.f32(c.capsule.height); w.f32(c.capsule.radius);
		w.i32(c.capsule.slices); w.i32(c.capsule.rings); break;
	case RenderShape::Plane:    w.vec2(c.plane.size); break;
	case RenderShape::Model:
	case RenderShape::ModelWires: w.u16(c.model.modelId); w.f32(c.model.scale); break;
	}
}
void deserialize(Reader& r, Renderable& c) {
	c.shape = RenderShape(r.u8());
	c.color = r.color();
	switch (c.shape) {
	case RenderShape::Cube:     c.cube.width = r.f32(); c.cube.height = r.f32(); c.cube.length = r.f32(); break;
	case RenderShape::Sphere:   c.sphere.radius = r.f32(); break;
	case RenderShape::Cylinder: c.cylinder.topRadius = r.f32(); c.cylinder.bottomRadius = r.f32();
		c.cylinder.height = r.f32(); c.cylinder.slices = r.i32(); break;
	case RenderShape::Capsule:  c.capsule.height = r.f32(); c.capsule.radius = r.f32();
		c.capsule.slices = r.i32(); c.capsule.rings = r.i32(); break;
	case RenderShape::Plane:    c.plane.size = r.vec2(); break;
	case RenderShape::Model:
	case RenderShape::ModelWires: c.model.modelId = r.u16(); c.model.scale = r.f32(); break;
	}
}

// ---- RigidBody2D ----
void serialize(Writer& w, const RigidBody2D& c) { w.u8(uint8_t(c.type)); w.f32(c.mass); w.vec2(c.velocity); }
void deserialize(Reader& r, RigidBody2D& c) { c.type = RbType(r.u8()); c.mass = r.f32(); c.velocity = r.vec2(); }

// ---- Collider2D ----
void serialize(Writer& w, const Collider2D& c) { writeRect(w, c.bounds); w.vec2(c.offset); }
void deserialize(Reader& r, Collider2D& c) { c.bounds = readRect(r); c.offset = r.vec2(); }

// ---- BoxCollider ----  (extents/center recomputed from bounds)
void serialize(Writer& w, const BoxCollider& c) { w.vec3(c.bounds.min); w.vec3(c.bounds.max); }
void deserialize(Reader& r, BoxCollider& c) {
	c.bounds.min = r.vec3();
	c.bounds.max = r.vec3();
	c.UpdateFromBounds();
}

// ---- MeshCollider ----  (geometry comes from the entity's model; rebuilt after load in step 4)
void serialize(Writer&, const MeshCollider&) {}
void deserialize(Reader&, MeshCollider&) {}