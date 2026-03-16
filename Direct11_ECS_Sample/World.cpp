#include "World.h"

Entity World::CreateEntity()
{
    if (!freeList_.empty())
    {
        const auto id = freeList_.back();
        freeList_.pop_back();
        return id;
    }

    return next_++;
}

void World::DestroyEntity(Entity entity)
{
    if (entity != InvalidEntity)
    {
        freeList_.push_back(entity);
    }
}

void World::AddSystem(std::shared_ptr<ISystem> system)
{
    systems_.push_back(std::move(system));
}

void World::UpdateSystems(float deltaTime)
{
    for (auto& system : systems_)
    {
        system->Update(*this, deltaTime);
    }
}

void World::ClearEntities()
{
    stores_.clear();
    freeList_.clear();
    next_ = 1;
}
