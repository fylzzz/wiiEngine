#include "RenderSystem.h"
#include "GL/gl.h"
#include <math.h>

void RenderSystem::SetDrawMode2D() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return;
}

void RenderSystem::SetDrawMode3D(Camera3D* camera) {
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

void RenderSystem::onEntityAdded(Entity e) {
    System::onEntityAdded(e);

    bool wants2D = world->hasComponent<PrimitiveRenderable2D>(e) || world->hasComponent<Renderable2D>(e) || world->hasComponent<Animator2D>(e);
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
}