#pragma once

#include "System.h"

class PhysicsSystem : public ISystem
{
public:
    void Update(World& world, float deltaTime) override;

private:
    static constexpr float Gravity = 600.0f;
};
