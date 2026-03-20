#pragma once

#include "Component.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

struct RenderComponent : public IComponent
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount = 0;
    DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};
