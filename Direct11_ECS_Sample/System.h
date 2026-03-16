#pragma once

class World;

class ISystem
{
public:
    virtual ~ISystem() = default;
    virtual void Update(World& world, float deltaTime) = 0;
};
