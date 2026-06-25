#include "RenderSystem.h"
#include "GL/gl.h"
#include <math.h>

void RenderSystem::onEntityAdded(Entity e) {
    System::onEntityAdded(e);

    bool wants2D = world->hasComponent<PrimitiveRenderable2D>(e) || world->hasComponent<Renderable2D>(e);
    bool alreadyInZMap = mEntityZIndex.count(e) > 0;

    if (wants2D && !alreadyInZMap) {
        int z = (int)world->getComponent<EngineTransform>(e).pos.z;
        mEntityZMap.insert({ z, e });
        mEntityZIndex[e] = z;
    }
}

void RenderSystem::onEntityRemoved(Entity e) {
    System::onEntityRemoved(e);

    auto it = mEntityZIndex.find(e);
    if (it != mEntityZIndex.end()) {
        auto range = mEntityZMap.equal_range(it->second);
        for (auto rit = range.first; rit != range.second; ++rit) {
            if (rit->second == e) {
                mEntityZMap.erase(rit);
                break;
            }
        }
        mEntityZIndex.erase(it);
    }
}

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
        else if (world->hasComponent<Animator2D>(e)) {
            auto& anim2D = world->getComponent<Animator2D>(e);

            auto it = anim2D.animClips.find(anim2D.currentAnim);
            if (it == anim2D.animClips.end() || it->second == INVALID_ANIM) continue;

            AnimationClip& clip = world->getAnim(it->second);
            if (clip.frames.empty()) continue;

            Rectangle src = clip.frames[anim2D.currentFrame];
            Rectangle dest = { trans.pos.x, trans.pos.y, src.width, src.height };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(clip.spritesheet, src, dest, origin, 0.0f, anim2D.color);
        }
    }

    for (auto& [z, e] : mEntityZMap) {
        auto& trans = world->getComponent<EngineTransform>(e);

        if (world->hasComponent<PrimitiveRenderable2D>(e)) {
            auto& primrender2D = world->getComponent<PrimitiveRenderable2D>(e);

            switch (primrender2D.shape) {
            case RenderShape2D::Rectangle:
                DrawRectangle(trans.pos.x, trans.pos.y, primrender2D.rectangle.width, primrender2D.rectangle.height, primrender2D.color);
                break;
            case RenderShape2D::Circle:
                DrawCircle(trans.pos.x + primrender2D.circle.radius, trans.pos.y + primrender2D.circle.radius, primrender2D.circle.radius, primrender2D.color);
                break;
            case RenderShape2D::Triangle:
                DrawTriangle(primrender2D.triangle.v1, primrender2D.triangle.v2, primrender2D.triangle.v3, primrender2D.color);
                break;
            case RenderShape2D::Ellipse:
                DrawEllipse(trans.pos.x, trans.pos.y, primrender2D.ellipse.radiusH, primrender2D.ellipse.radiusV, primrender2D.color);
                break;
            }
        }
        else if (world->hasComponent<Renderable2D>(e)) {
            auto& render2D = world->getComponent<Renderable2D>(e);

            if (render2D.spriteId == INVALID_SPRITE) continue;
            DrawTexture(render2D.texture, (int)trans.pos.x, (int)trans.pos.y, render2D.color);
        }
    }
}