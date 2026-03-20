#include "CollisionSystem.h"
#include "World.h"
#include "TransformComponent.h"
#include "RigidBodyComponent.h"
#include "BoxColliderComponent.h"

#include <cmath>

void CollisionSystem::Update(World& world, float deltaTime)
{
    auto& transforms = world.GetStore<TransformComponent>();
    auto& colliders = world.GetStore<BoxColliderComponent>();
    auto& rigidBodies = world.GetStore<RigidBodyComponent>();

    for (auto& [dynamicEntity, dynamicRb] : rigidBodies.All())
    {
        auto* dynamicTransform = transforms.TryGet(dynamicEntity);
        auto* dynamicCollider = colliders.TryGet(dynamicEntity);
        if (!dynamicTransform || !dynamicCollider)
        {
            continue;
        }

        if (dynamicCollider->isStatic)
        {
            continue;
        }

        float dLeft   = dynamicTransform->position.x - dynamicCollider->width * 0.5f;
        float dRight  = dynamicTransform->position.x + dynamicCollider->width * 0.5f;
        float dTop    = dynamicTransform->position.y - dynamicCollider->height * 0.5f;
        float dBottom = dynamicTransform->position.y + dynamicCollider->height * 0.5f;

        for (auto& [staticEntity, staticCollider] : colliders.All())
        {
            if (staticEntity == dynamicEntity)
            {
                continue;
            }

            if (!staticCollider.isStatic)
            {
                continue;
            }

            auto* staticTransform = transforms.TryGet(staticEntity);
            if (!staticTransform)
            {
                continue;
            }

            float sLeft   = staticTransform->position.x - staticCollider.width * 0.5f;
            float sRight  = staticTransform->position.x + staticCollider.width * 0.5f;
            float sTop    = staticTransform->position.y - staticCollider.height * 0.5f;
            float sBottom = staticTransform->position.y + staticCollider.height * 0.5f;

            float overlapX = (std::min)(dRight, sRight) - (std::max)(dLeft, sLeft);
            float overlapY = (std::min)(dBottom, sBottom) - (std::max)(dTop, sTop);

            if (overlapX <= 0.0f || overlapY <= 0.0f)
            {
                continue;
            }

            if (overlapX < overlapY)
            {
                if (dynamicTransform->position.x < staticTransform->position.x)
                {
                    dynamicTransform->position.x -= overlapX;
                }
                else
                {
                    dynamicTransform->position.x += overlapX;
                }
                dynamicRb.velocity.x = 0.0f;
            }
            else
            {
                if (dynamicTransform->position.y < staticTransform->position.y)
                {
                    dynamicTransform->position.y -= overlapY;
                    if (dynamicRb.velocity.y > 0.0f)
                    {
                        dynamicRb.velocity.y = 0.0f;
                    }
                }
                else
                {
                    dynamicTransform->position.y += overlapY;
                    if (dynamicRb.velocity.y < 0.0f)
                    {
                        dynamicRb.velocity.y = 0.0f;
                    }
                }
            }

            dLeft   = dynamicTransform->position.x - dynamicCollider->width * 0.5f;
            dRight  = dynamicTransform->position.x + dynamicCollider->width * 0.5f;
            dTop    = dynamicTransform->position.y - dynamicCollider->height * 0.5f;
            dBottom = dynamicTransform->position.y + dynamicCollider->height * 0.5f;
        }
    }
}
