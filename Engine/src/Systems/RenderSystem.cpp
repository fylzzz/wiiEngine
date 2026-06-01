#include "RenderSystem.h"
#include "GL/gl.h"
#include <math.h>

void RenderSystem::update(float dt) {
    
    for (Entity e : mEntities) {
        auto& trans = world->getComponent<EngineTransform>(e);

        if (world->hasComponent<Renderable>(e)) {
            auto& render = world->getComponent<Renderable>(e);

            switch (render.shape) {
            case RenderShape::Cube: {
                DrawCube(trans.pos, render.cube.width, render.cube.height, render.cube.length, render.color);
                break;
            }
            case RenderShape::Sphere: {
                DrawSphere(trans.pos, render.sphere.radius, render.color);
                break;
            }
            case RenderShape::Cylinder: {
                DrawCylinder(trans.pos, render.cylinder.topRadius, render.cylinder.bottomRadius, render.cylinder.height, render.cylinder.slices, render.color);
                break;
            }
            case RenderShape::Capsule: {
                Vector3 start = { trans.pos.x, trans.pos.y, trans.pos.z };
                Vector3 end = { trans.pos.x, trans.pos.y + render.capsule.height, trans.pos.z };
                DrawCapsule(start, end, render.capsule.radius, render.capsule.slices, render.capsule.rings, render.color);
                break;
            }
            case RenderShape::Plane: {
                DrawPlane(trans.pos, render.plane.size, render.color);
                break;
            }
            case RenderShape::Model: {
                if (render.model.modelId == INVALID_RESOURCE) continue;
                Model& model = world->getModel(render.model.modelId);
                DrawModel(model, trans.pos, render.model.scale, render.color);
                break;
            }
            case RenderShape::ModelWires: {
                if (render.model.modelId == INVALID_RESOURCE) continue;
                Model& model = world->getModel(render.model.modelId);
                DrawModelWires(model, trans.pos, render.model.scale, render.color);
                break;
            }
            }
        }
        else if (world->hasComponent<Renderable2D>(e)) {
            auto& render2D = world->getComponent<Renderable2D>(e);

            if (render2D.spriteId == INVALID_RESOURCE) continue;
                DrawTexture(render2D.texture, (int)trans.pos.x, (int)trans.pos.y, render2D.color);
            }
        }
}