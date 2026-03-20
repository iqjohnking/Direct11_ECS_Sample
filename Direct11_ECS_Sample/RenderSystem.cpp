#include "RenderSystem.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GrappleComponent.h"
#include "DragComponent.h"

#include <d3dcompiler.h>
#include <CommonStates.h>
#include <WICTextureLoader.h>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

struct VertexPos
{
    DirectX::XMFLOAT3 position;
};

bool RenderSystem::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
                              UINT screenWidth, UINT screenHeight)
{
    device_ = device;
    context_ = context;
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;

    projection_ = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, static_cast<float>(screenWidth),
        static_cast<float>(screenHeight), 0.0f,
        0.0f, 1.0f);

    const char* vsSource =
        "cbuffer CB : register(b0) {\n"
        "    float4x4 wvp;\n"
        "    float4 color;\n"
        "};\n"
        "struct VS_IN  { float3 pos : POSITION; };\n"
        "struct VS_OUT { float4 pos : SV_POSITION; float4 col : COLOR; };\n"
        "VS_OUT main(VS_IN input) {\n"
        "    VS_OUT output;\n"
        "    output.pos = mul(float4(input.pos, 1.0f), wvp);\n"
        "    output.col = color;\n"
        "    return output;\n"
        "}\n";

    const char* psSource =
        "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };\n"
        "float4 main(PS_IN input) : SV_TARGET {\n"
        "    return input.col;\n"
        "}\n";

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    if (FAILED(D3DCompile(vsSource, strlen(vsSource), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, vertexShader_.GetAddressOf())))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    if (FAILED(D3DCompile(psSource, strlen(psSource), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, pixelShader_.GetAddressOf())))
    {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (FAILED(device_->CreateInputLayout(layoutDesc, static_cast<UINT>(std::size(layoutDesc)),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout_.GetAddressOf())))
    {
        return false;
    }

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBufferData);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device_->CreateBuffer(&cbDesc, nullptr, constantBuffer_.GetAddressOf())))
    {
        return false;
    }

    primitiveBatch_ = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context_);

    basicEffect_ = std::make_unique<DirectX::BasicEffect>(device_);
    basicEffect_->SetVertexColorEnabled(true);
    basicEffect_->SetProjection(projection_);
    basicEffect_->SetView(DirectX::XMMatrixIdentity());
    basicEffect_->SetWorld(DirectX::XMMatrixIdentity());

    void const* shaderByteCode = nullptr;
    size_t byteCodeLength = 0;
    basicEffect_->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    if (FAILED(device_->CreateInputLayout(
        DirectX::VertexPositionColor::InputElements,
        DirectX::VertexPositionColor::InputElementCount,
        shaderByteCode, byteCodeLength,
        primitiveInputLayout_.GetAddressOf())))
    {
        return false;
    }

    spriteBatch_ = std::make_unique<DirectX::SpriteBatch>(context_);
    commonStates_ = std::make_unique<DirectX::CommonStates>(device_);

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring exeDir(exePath);
    exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\/") + 1);

    std::wstring candidates[] = {
        exeDir + L"assets\\number.png",
        exeDir + L"..\\assets\\number.png",
        exeDir + L"..\\..\\assets\\number.png",
        exeDir + L"..\\..\\DirectX11OneButton\\assets\\number.png",
    };

    for (auto& path : candidates)
    {
        if (SUCCEEDED(DirectX::CreateWICTextureFromFile(
            device_, path.c_str(), nullptr, numberSRV_.GetAddressOf())))
        {
            break;
        }
    }

    // --- Background layers ---
    const wchar_t* bgFiles[] = {
        L"assets\\Free Pixel Art Hill\\PNG\\Hills Layer 01.png",
        L"assets\\Free Pixel Art Hill\\PNG\\Hills Layer 02.png",
        L"assets\\Free Pixel Art Hill\\PNG\\Hills Layer 03.png",
        L"assets\\Free Pixel Art Hill\\PNG\\Hills Layer 04.png",
    };

    for (int i = 0; i < BgLayerCount_; ++i)
    {
        for (auto& base : candidates)
        {
            std::wstring dir = exeDir;
            std::wstring paths[] = {
                dir + bgFiles[i],
                dir + L"..\\" + bgFiles[i],
                dir + L"..\\..\\" + bgFiles[i],
                dir + L"..\\..\\DirectX11OneButton\\" + bgFiles[i],
            };

            bool loaded = false;
            for (auto& p : paths)
            {
                if (SUCCEEDED(DirectX::CreateWICTextureFromFile(
                    device_, p.c_str(), nullptr, bgLayerSRV_[i].GetAddressOf())))
                {
                    loaded = true;
                    break;
                }
            }
            if (loaded) break;
        }
    }

    return true;
}

void RenderSystem::Update(World& world, float deltaTime)
{
    DrawEntities(world);
    DrawRopes(world);
    DrawDragIndicators(world);
}

void RenderSystem::DrawEntities(World& world)
{
    auto& transforms = world.GetStore<TransformComponent>();
    auto& renders = world.GetStore<RenderComponent>();
    auto& drags = world.GetStore<DragComponent>();
    auto& grapples = world.GetStore<GrappleComponent>();

    context_->IASetInputLayout(inputLayout_.Get());
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

    for (auto& [entity, render] : renders.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform)
        {
            continue;
        }

        DirectX::XMMATRIX world_matrix = transform->GetWorldMatrix();
        DirectX::XMMATRIX wvp = DirectX::XMMatrixTranspose(world_matrix * view_ * projection_);

        DirectX::XMFLOAT4 drawColor = render.color;

        auto* grapple = grapples.TryGet(entity);
        auto* drag = drags.TryGet(entity);
        bool isSwinging = grapple && grapple->isActive;

        if (drag && drag->boostFlashTimer > 0.0f && isSwinging)
        {
            float t = drag->boostFlashTimer / 0.15f;
            drawColor = {
                1.0f,
                1.0f,
                1.0f,
                1.0f
            };
        }
        else if (isSwinging && drag && drag->isDragging)
        {
            drawColor = { 0.2f, 0.4f, 1.0f, 1.0f };
        }
        else if (isSwinging)
        {
            drawColor = { 0.2f, 0.9f, 0.3f, 1.0f };
        }

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (SUCCEEDED(context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            auto* cb = static_cast<ConstantBufferData*>(mapped.pData);
            DirectX::XMStoreFloat4x4(&cb->worldViewProjection, wvp);
            cb->color = drawColor;
            context_->Unmap(constantBuffer_.Get(), 0);
        }

        const UINT stride = sizeof(VertexPos);
        const UINT offset = 0;
        context_->IASetVertexBuffers(0, 1, render.vertexBuffer.GetAddressOf(), &stride, &offset);
        context_->IASetIndexBuffer(render.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        context_->DrawIndexed(render.indexCount, 0, 0);
    }
}

void RenderSystem::DrawRopes(World& world)
{
    auto& transforms = world.GetStore<TransformComponent>();
    auto& grapples = world.GetStore<GrappleComponent>();

    bool hasActiveLine = false;
    for (auto& [entity, grapple] : grapples.All())
    {
        if ((grapple.isActive || grapple.isShooting) && transforms.TryGet(entity))
        {
            hasActiveLine = true;
            break;
        }
    }

    if (!hasActiveLine)
    {
        return;
    }

    basicEffect_->SetView(view_);
    basicEffect_->Apply(context_);
    context_->IASetInputLayout(primitiveInputLayout_.Get());

    primitiveBatch_->Begin();

    for (auto& [entity, grapple] : grapples.All())
    {
        auto* transform = transforms.TryGet(entity);
        if (!transform)
        {
            continue;
        }

        DirectX::SimpleMath::Vector2 playerPos(
            transform->position.x, transform->position.y);

        if (grapple.isActive)
        {
            // Connected rope: player to anchor (white)
            DirectX::VertexPositionColor v1(
                DirectX::XMFLOAT3(grapple.anchor.x, grapple.anchor.y, 0.0f),
                DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

            DirectX::VertexPositionColor v2(
                DirectX::XMFLOAT3(playerPos.x, playerPos.y, 0.0f),
                DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

            primitiveBatch_->DrawLine(v1, v2);
        }
        else if (grapple.isShooting)
        {
            // Shooting animation: player to extending tip
            DirectX::SimpleMath::Vector2 tipPos =
                playerPos + grapple.shootDir * grapple.currentExtent;

            // Line color: yellow for miss, green for will-hit
            //DirectX::XMFLOAT4 lineColor = grapple.didHit
            //    ? DirectX::XMFLOAT4(0.2f, 1.0f, 0.4f, 1.0f)
            //    : DirectX::XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f);
            DirectX::XMFLOAT4 lineColor = DirectX::XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f);

            DirectX::VertexPositionColor v1(
                DirectX::XMFLOAT3(playerPos.x, playerPos.y, 0.0f),
                lineColor);

            DirectX::VertexPositionColor v2(
                DirectX::XMFLOAT3(tipPos.x, tipPos.y, 0.0f),
                lineColor);

            primitiveBatch_->DrawLine(v1, v2);
        }
    }

    primitiveBatch_->End();
}

RenderComponent RenderSystem::CreateQuadMesh(float width, float height,
                                             const DirectX::XMFLOAT4& color) const
{
    const float hw = width * 0.5f;
    const float hh = height * 0.5f;

    VertexPos vertices[] =
    {
        { { -hw,  -hh, 0.0f } },
        { {  hw,  -hh, 0.0f } },
        { {  hw,   hh, 0.0f } },
        { { -hw,   hh, 0.0f } },
    };

    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

    RenderComponent rc;
    rc.color = color;
    rc.indexCount = static_cast<UINT>(std::size(indices));

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    device_->CreateBuffer(&vbDesc, &vbData, rc.vertexBuffer.GetAddressOf());

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    device_->CreateBuffer(&ibDesc, &ibData, rc.indexBuffer.GetAddressOf());

    return rc;
}

void RenderSystem::DrawDragIndicators(World& world)
{
    auto& transforms = world.GetStore<TransformComponent>();
    auto& drags = world.GetStore<DragComponent>();

    bool hasActive = false;
    for (auto& [entity, drag] : drags.All())
    {
        if (drag.isDragging && transforms.TryGet(entity))
        {
            hasActive = true;
            break;
        }
    }

    if (!hasActive)
    {
        return;
    }

    basicEffect_->SetView(view_);
    basicEffect_->Apply(context_);
    context_->IASetInputLayout(primitiveInputLayout_.Get());

    primitiveBatch_->Begin();

    for (auto& [entity, drag] : drags.All())
    {
        if (!drag.isDragging)
        {
            continue;
        }

        auto* transform = transforms.TryGet(entity);
        if (!transform)
        {
            continue;
        }

        DirectX::SimpleMath::Vector2 pos(transform->position.x, transform->position.y);
        DirectX::SimpleMath::Vector2 impulse = drag.launchImpulse;
        float impulseLen = impulse.Length();
        if (impulseLen < 1e-4f)
        {
            continue;
        }

        DirectX::SimpleMath::Vector2 dir = impulse / impulseLen;
        float arrowLen = impulseLen * 0.3f;

        constexpr int segmentCount = 10;
        float segLen = arrowLen / segmentCount;

        for (int i = 0; i < segmentCount; i += 2)
        {
            DirectX::SimpleMath::Vector2 p0 = pos + dir * (segLen * i);
            DirectX::SimpleMath::Vector2 p1 = pos + dir * (segLen * (i + 1));

            DirectX::VertexPositionColor v0(
                DirectX::XMFLOAT3(p0.x, p0.y, 0.0f),
                DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

            DirectX::VertexPositionColor v1(
                DirectX::XMFLOAT3(p1.x, p1.y, 0.0f),
                DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

            primitiveBatch_->DrawLine(v0, v1);
        }
    }

    primitiveBatch_->End();
}

void RenderSystem::SetViewMatrix(const DirectX::XMMATRIX& view)
{
    view_ = view;
}

void RenderSystem::Resize(UINT newWidth, UINT newHeight)
{
    if (newWidth == 0 || newHeight == 0) return;

    screenWidth_ = newWidth;
    screenHeight_ = newHeight;

    projection_ = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f, static_cast<float>(newWidth),
        static_cast<float>(newHeight), 0.0f,
        0.0f, 1.0f);

    if (basicEffect_)
    {
        basicEffect_->SetProjection(projection_);
    }
}

void RenderSystem::DrawScoreRightAligned(int score, float rightX, float topY, float scale)
{
    if (!numberSRV_ || !spriteBatch_) return;

    if (score < 0) score = 0;
    std::string digits = std::to_string(score);

    float digitW = NumDigitW_ * scale;

    spriteBatch_->Begin(DirectX::SpriteSortMode_Deferred, commonStates_->NonPremultiplied());

    float x = rightX - digitW;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i)
    {
        int d = digits[i] - '0';
        RECT src = { d * NumDigitW_, 0, (d + 1) * NumDigitW_, NumDigitH_ };

        spriteBatch_->Draw(numberSRV_.Get(),
            DirectX::XMFLOAT2(x, topY),
            &src,
            DirectX::Colors::White,
            0.0f,
            DirectX::XMFLOAT2(0.0f, 0.0f),
            scale);

        x -= digitW;
    }

    spriteBatch_->End();
}

void RenderSystem::DrawScoreCentered(int score, float centerX, float centerY, float scale)
{
    if (!numberSRV_ || !spriteBatch_) return;

    if (score < 0) score = 0;
    std::string digits = std::to_string(score);

    float digitW = NumDigitW_ * scale;
    float digitH = NumDigitH_ * scale;
    float totalW = digitW * digits.size();

    float startX = centerX - totalW * 0.5f;
    float y = centerY - digitH * 0.5f;

    spriteBatch_->Begin(DirectX::SpriteSortMode_Deferred, commonStates_->NonPremultiplied());

    for (size_t i = 0; i < digits.size(); ++i)
    {
        int d = digits[i] - '0';
        RECT src = { d * NumDigitW_, 0, (d + 1) * NumDigitW_, NumDigitH_ };

        spriteBatch_->Draw(numberSRV_.Get(),
            DirectX::XMFLOAT2(startX + digitW * i, y),
            &src,
            DirectX::Colors::White,
            0.0f,
            DirectX::XMFLOAT2(0.0f, 0.0f),
            scale);
    }

    spriteBatch_->End();
}

void RenderSystem::DrawBackground(float cameraX, float cameraY)
{
    if (!spriteBatch_) return;

    float sw = static_cast<float>(screenWidth_);
    float sh = static_cast<float>(screenHeight_);

    float scaleY = sh / static_cast<float>(BgTexH_);
    float scaledW = static_cast<float>(BgTexW_) * scaleY;

    spriteBatch_->Begin(DirectX::SpriteSortMode_Deferred, commonStates_->NonPremultiplied());

    for (int layer = 0; layer < BgLayerCount_; ++layer)
    {
        if (!bgLayerSRV_[layer]) continue;

        float speed = BgParallax_[layer];
        float rawOffset = -cameraX * speed;

        float offset = fmodf(rawOffset, scaledW);
        if (offset > 0.0f) offset -= scaledW;

        float x = offset;
        while (x < sw)
        {
            RECT src = { 0, 0, BgTexW_, BgTexH_ };
            spriteBatch_->Draw(bgLayerSRV_[layer].Get(),
                DirectX::XMFLOAT2(x, 0.0f),
                &src,
                DirectX::Colors::White,
                0.0f,
                DirectX::XMFLOAT2(0.0f, 0.0f),
                DirectX::XMFLOAT2(scaleY, scaleY));
            x += scaledW;
        }
    }

    spriteBatch_->End();
}
