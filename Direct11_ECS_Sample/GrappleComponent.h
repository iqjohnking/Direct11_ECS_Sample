#pragma once

#include "Component.h"
#include <SimpleMath.h>

struct GrappleComponent : public IComponent
{
    // Active swing state
    DirectX::SimpleMath::Vector2 anchor = {};
    float ropeLength = 0.0f;
    bool isActive = false;

    // Shooting animation state
    bool isShooting = false;
    bool didHit = false;
    float shootTimer = 0.0f;
    float shootSpeed = 2000.0f;
    float maxDistance = 500.0f;
    DirectX::SimpleMath::Vector2 shootOrigin = {};
    DirectX::SimpleMath::Vector2 shootDir = {};
    DirectX::SimpleMath::Vector2 hitPoint = {};
    float hitDistance = 0.0f;
    float currentExtent = 0.0f;
};
