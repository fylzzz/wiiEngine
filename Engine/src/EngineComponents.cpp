#include "EngineComponents.h"
#include "Components.h"


template<typename T>
static void reg(World& w) {
	w.registerComponent<T>();
	w.registerComponentSerializer<T>();
}

void RegisterEngineComponents(World& w)
{
	// APPEND ONLY. Reordering changes every saved scene's meaning.
	reg<EngineTransform>(w);
	reg<Renderable>(w);
	reg<Renderable2D>(w);
	reg<PrimitiveRenderable2D>(w);
	reg<Collider2D>(w);
	reg<BoxCollider>(w);
	reg<MeshCollider>(w);
	reg<RigidBody2D>(w);
	reg<Animator2D>(w);
}

void ApplyComponentMethods(World& w, Entity e, Signature sig)
{
	if (!sig.test(w.getComponentType<Renderable>())) return;

	const Renderable& rend = w.getComponent<Renderable>(e);
	bool isModel = rend.shape == RenderShape::Model || rend.shape == RenderShape::ModelWires;
	if (!isModel || rend.model.modelId == INVALID_RESOURCE) return;

	if (sig.test(w.getComponentType<BoxCollider>())) {
		BoxCollider& box = w.getComponent<BoxCollider>(e);
		box.bounds = w.getModelBoundingBox(rend.model.modelId);
		box.UpdateFromBounds();
	}
	if (sig.test(w.getComponentType<MeshCollider>())) {
		MeshCollider& mesh = w.getComponent<MeshCollider>(e);
		mesh.localVertices = mesh.GetModelVertices(w.getModel(rend.model.modelId));
	}
}