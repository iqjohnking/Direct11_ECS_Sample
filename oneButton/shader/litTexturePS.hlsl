#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// ピクセルシェーダーのエントリポイント
float4 main(in PS_IN input) : SV_Target
{
    float4 color;
    
    
    if (Material.TextureEnable)
    {
        // TextureEnable が true のときはテクスチャ色を採用し、頂点カラー(input.col)でティント（色味/α）を反映する。
        color = g_Texture.Sample(g_SamplerState, input.tex);
        color *= input.col;
        color *= Material.Diffuse;
    }
    else
    {
        // TextureEnable が false のときはマテリアルの Diffuse 色をベースにし、同様に頂点カラーを乗算する。
        color = Material.Diffuse * input.col;
    }
    return color;
}

