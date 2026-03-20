#pragma once

#include "Component.h"
#include <SimpleMath.h>

struct CameraComponent : public IComponent
{
    DirectX::SimpleMath::Vector2 targetPosition = {};
    DirectX::SimpleMath::Vector2 currentPosition = {};
    float lerpSpeed = 5.0f;
};
