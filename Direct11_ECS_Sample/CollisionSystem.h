#pragma once

#include "System.h"

class CollisionSystem : public ISystem
{
public:
    void Update(World& world, float deltaTime) override;
};
