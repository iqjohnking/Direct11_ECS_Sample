#pragma once

#include "Component.h"
#include <SimpleMath.h>

struct DragComponent : public IComponent
{
    // Right-click slingshot
    bool isDragging = false;
    bool shouldLaunch = false;
    DirectX::SimpleMath::Vector2 dragStartWorld = {};
    DirectX::SimpleMath::Vector2 dragCurrentWorld = {};
    DirectX::SimpleMath::Vector2 launchImpulse = {};
    float maxDragLength = 400.0f;
    float impulseStrength = 2.5f;

    // Mouse swing acceleration (raw per-frame delta, not velocity)
    DirectX::SimpleMath::Vector2 mouseDelta = {};
    float swingSensitivity = 0.15f;
    float maxSwingForce = 250.0f;

    // Visual feedback
    float boostFlashTimer = 0.0f;
};
