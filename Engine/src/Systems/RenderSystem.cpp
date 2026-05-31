#include "RenderSystem.h"
#include "GL/gl.h"
#include <math.h>

void RenderSystem::update(float dt) {
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

    for (Entity e : mEntities) {
        auto& trans = world->getComponent<EngineTransform>(e);
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