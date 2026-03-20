#pragma once

#include "System.h"
#include "Entity.h"

#include <vector>
#include <memory>

class RenderSystem;

class LevelSystem : public ISystem
{
public:
    void Initialize(const std::shared_ptr<RenderSystem>& renderSystem,
                    Entity playerEntity);
    void Update(World& world, float deltaTime) override;

private:
    void SpawnBlock(World& world, float x, float y, float w, float h);
    void CleanupBlocks(World& world, float playerX);

    std::shared_ptr<RenderSystem> renderSystem_;
    Entity playerEntity_ = 0;
    std::vector<Entity> blocks_;
    float lastSpawnX_ = 0.0f;
    float spawnInterval_ = 300.0f;
    float cleanupMargin_ = 800.0f;
};
