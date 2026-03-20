#include "LevelSystem.h"
#include "RenderSystem.h"
#include "World.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "BoxColliderComponent.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

void LevelSystem::Initialize(const std::shared_ptr<RenderSystem>& renderSystem,
                              Entity playerEntity)
{
    renderSystem_ = renderSystem;
    playerEntity_ = playerEntity;
    blocks_.clear();
    lastSpawnX_ = 0.0f;
}

void LevelSystem::SpawnBlock(World& world, float x, float y, float w, float h)
{
    Entity block = world.CreateEntity();

    auto& bt = world.AddComponent<TransformComponent>(block);
    bt.position = { x, y, 0.0f };
    bt.scale = { 1.0f, 1.0f };

    DirectX::XMFLOAT4 blockColor = { 0.3f, 0.6f, 0.9f, 1.0f };
    world.AddComponent<RenderComponent>(
        block,
        renderSystem_->CreateQuadMesh(w, h, blockColor));

    auto& bc = world.AddComponent<BoxColliderComponent>(block);
    bc.width = w;
    bc.height = h;
    bc.isStatic = true;

    blocks_.push_back(block);
}

void LevelSystem::CleanupBlocks(World& world, float playerX)
{
    float removeThreshold = playerX - cleanupMargin_;

    auto it = blocks_.begin();
    while (it != blocks_.end())
    {
        auto* transform = world.GetComponent<TransformComponent>(*it);
        if (transform && transform->position.x < removeThreshold)
        {
            world.RemoveComponent<TransformComponent>(*it);
            world.RemoveComponent<RenderComponent>(*it);
            world.RemoveComponent<BoxColliderComponent>(*it);
            world.DestroyEntity(*it);
            it = blocks_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LevelSystem::Update(World& world, float deltaTime)
{
    auto* playerTransform = world.GetComponent<TransformComponent>(playerEntity_);
    if (!playerTransform)
    {
        return;
    }

    float playerX = playerTransform->position.x;

    float generateAhead = playerX + 1200.0f;
    while (lastSpawnX_ < generateAhead)
    {
        lastSpawnX_ += spawnInterval_;

        float randomY = 200.0f + static_cast<float>(rand() % 300);
        float randomW = 120.0f + static_cast<float>(rand() % 80);
        float randomH = 25.0f + static_cast<float>(rand() % 15);

        SpawnBlock(world, lastSpawnX_, randomY, randomW, randomH);
    }

    CleanupBlocks(world, playerX);
}
