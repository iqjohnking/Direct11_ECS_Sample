#pragma once

#include "System.h"
#include "World.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Effects.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <memory>
#include <string>

struct RenderComponent;

struct alignas(16) ConstantBufferData
{
    DirectX::XMFLOAT4X4 worldViewProjection;
    DirectX::XMFLOAT4 color;
};

class RenderSystem : public ISystem
{
public:
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
                    UINT screenWidth, UINT screenHeight);

    void Update(World& world, float deltaTime) override;

    void SetViewMatrix(const DirectX::XMMATRIX& view);
    void Resize(UINT newWidth, UINT newHeight);

    DirectX::XMMATRIX GetProjectionMatrix() const { return projection_; }
    DirectX::XMMATRIX GetViewMatrix() const { return view_; }
    UINT GetScreenWidth() const { return screenWidth_; }
    UINT GetScreenHeight() const { return screenHeight_; }

    RenderComponent CreateQuadMesh(float width, float height,
                                   const DirectX::XMFLOAT4& color) const;

    void DrawScoreRightAligned(int score, float rightX, float topY, float scale = 1.0f);
    void DrawScoreCentered(int score, float centerX, float centerY, float scale = 1.0f);

    void DrawBackground(float cameraX, float cameraY);

private:
void DrawEntities(World& world);
void DrawRopes(World& world);
void DrawDragIndicators(World& world);

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    UINT screenWidth_ = 0;
    UINT screenHeight_ = 0;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> primitiveBatch_;
    std::unique_ptr<DirectX::BasicEffect> basicEffect_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> primitiveInputLayout_;

    std::unique_ptr<DirectX::SpriteBatch> spriteBatch_;
    std::unique_ptr<DirectX::CommonStates> commonStates_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> numberSRV_;
    static constexpr int NumDigitW_ = 130;
    static constexpr int NumDigitH_ = 179;

    static constexpr int BgLayerCount_ = 4;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bgLayerSRV_[BgLayerCount_];
    static constexpr int BgTexW_ = 512;
    static constexpr int BgTexH_ = 256;
    static constexpr float BgParallax_[BgLayerCount_] = { 0.05f, 0.15f, 0.35f, 0.6f };

    DirectX::XMMATRIX projection_ = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX view_ = DirectX::XMMatrixIdentity();
};
