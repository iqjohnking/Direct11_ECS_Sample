#pragma once

#include "Component.h"
#include <SimpleMath.h>

struct TransformComponent : public IComponent
{
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
    float rotation = 0.0f;
    DirectX::SimpleMath::Vector2 scale = DirectX::SimpleMath::Vector2::One;

    DirectX::XMMATRIX GetWorldMatrix() const
    {
        return DirectX::XMMatrixScaling(scale.x, scale.y, 1.0f)
             * DirectX::XMMatrixRotationZ(rotation)
             * DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    }
};
