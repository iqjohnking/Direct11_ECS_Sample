#pragma once

#include "Component.h"
#include <SimpleMath.h>

struct RigidBodyComponent : public IComponent
{
    DirectX::SimpleMath::Vector2 velocity = {};
    DirectX::SimpleMath::Vector2 acceleration = {};
    float gravityScale = 1.0f;
};
