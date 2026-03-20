#pragma once

#include "System.h"
#include <SimpleMath.h>

class CameraSystem : public ISystem
{
public:
    void Update(World& world, float deltaTime) override;

    DirectX::XMMATRIX GetViewMatrix() const { return viewMatrix_; }

private:
    DirectX::XMMATRIX viewMatrix_ = DirectX::XMMatrixIdentity();
};
