#include "CameraSystem.h"
#include "World.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

void CameraSystem::Update(World& world, float deltaTime)
{
    auto& cameras = world.GetStore<CameraComponent>();
    auto& transforms = world.GetStore<TransformComponent>();

    for (auto& [entity, camera] : cameras.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform)
        {
            continue;
        }

        camera.targetPosition = DirectX::SimpleMath::Vector2(
            transform->position.x, transform->position.y);

        float t = 1.0f - expf(-camera.lerpSpeed * deltaTime);
        camera.currentPosition = DirectX::SimpleMath::Vector2::Lerp(
            camera.currentPosition, camera.targetPosition, t);

        float offsetX = camera.currentPosition.x;
        float offsetY = camera.currentPosition.y;

        viewMatrix_ = DirectX::XMMatrixTranslation(-offsetX, -offsetY, 0.0f);
    }
}
