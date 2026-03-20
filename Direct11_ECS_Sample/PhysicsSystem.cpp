#include "PhysicsSystem.h"
#include "World.h"
#include "TransformComponent.h"
#include "RigidBodyComponent.h"
#include "GrappleComponent.h"
#include "DragComponent.h"

void PhysicsSystem::Update(World& world, float deltaTime)
{
    auto& transforms = world.GetStore<TransformComponent>();
    auto& rigidBodies = world.GetStore<RigidBodyComponent>();
    auto& grapples = world.GetStore<GrappleComponent>();
    auto& drags = world.GetStore<DragComponent>();

    for (auto& [entity, rb] : rigidBodies.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform)
        {
            continue;
        }

        // --- Apply drag impulse (only while swinging) ---
        auto* drag = drags.TryGet(entity);
        auto* grapple = grapples.TryGet(entity);
        using namespace DirectX::SimpleMath;

        if (drag && drag->shouldLaunch && grapple && grapple->isActive)
        {
            rb.velocity.x += drag->launchImpulse.x;
            rb.velocity.y += drag->launchImpulse.y;
            drag->shouldLaunch = false;
            drag->launchImpulse = {};
        }
        else if (drag && drag->shouldLaunch)
        {
            drag->shouldLaunch = false;
            drag->launchImpulse = {};
        }

        // Defensive cleanup: if grapple is inactive, ensure drag state is reset
        if (drag && grapple && !grapple->isActive)
        {
            if (drag->isDragging)
            {
                drag->isDragging = false;
                drag->shouldLaunch = false;
                drag->launchImpulse = {};
            }
            drag->mouseDelta = {};
        }

        if (grapple && grapple->isActive)
        {
            Vector2 pos(transform->position.x, transform->position.y);

            // --- Mouse swing acceleration ---
            if (drag)
            {
                Vector2 ropeVec = pos - grapple->anchor;
                float ropeDist = ropeVec.Length();
                float mouseLen = drag->mouseDelta.Length();

                if (ropeDist > 0.001f && mouseLen > 0.01f)
                {
                    Vector2 ropeDir = ropeVec / ropeDist;
                    Vector2 tangent(-ropeDir.y, ropeDir.x);

                    // Direction the mouse moved (normalized)
                    Vector2 mouseDeltaDir = drag->mouseDelta / mouseLen;

                    // How well does mouse direction align with swing tangent?
                    // positive = same direction as swing, negative = opposite
                    float alignment = mouseDeltaDir.Dot(tangent);

                    // Scale: raw pixel delta * sensitivity, clamped
                    float rawForce = mouseLen * drag->swingSensitivity * alignment;
                    float maxF = drag->maxSwingForce;
                    if (rawForce > maxF) rawForce = maxF;
                    if (rawForce < -maxF) rawForce = -maxF;

                    // Boost at bottom of swing (rope pointing down)
                    float downDot = ropeDir.Dot(Vector2(0.0f, 1.0f));
                    float bottomBoost = 0.5f + 0.5f * (std::max)(0.0f, downDot);

                    float appliedForce = rawForce * bottomBoost;
                    rb.velocity += tangent * appliedForce;

                    // Trigger flash if significant boost
                    if (fabsf(appliedForce) > 20.0f)
                    {
                        drag->boostFlashTimer = 0.15f;
                    }
                }
            }

            // Apply gravity normally
            rb.velocity.y += Gravity * rb.gravityScale * deltaTime;
            pos.x += rb.velocity.x * deltaTime;
            pos.y += rb.velocity.y * deltaTime;

            // Rope constraint: only pull back if beyond rope length (allow slack)
            Vector2 ropeVec = pos - grapple->anchor;
            float dist = ropeVec.Length();

            if (dist > grapple->ropeLength && dist > 0.001f)
            {
                Vector2 ropeDir = ropeVec / dist;
                pos = grapple->anchor + ropeDir * grapple->ropeLength;

                // Project velocity onto tangent (remove radial outward component)
                Vector2 tangent(-ropeDir.y, ropeDir.x);
                float tangentialSpeed = rb.velocity.Dot(tangent);
                tangentialSpeed *= 0.999f;
                rb.velocity = tangent * tangentialSpeed;
            }

            transform->position.x = pos.x;
            transform->position.y = pos.y;
        }
        else
        {
            rb.velocity.y += Gravity * rb.gravityScale * deltaTime;
            transform->position.x += rb.velocity.x * deltaTime;
            transform->position.y += rb.velocity.y * deltaTime;
        }
    }
}
