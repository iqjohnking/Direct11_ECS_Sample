#include "common.hlsl"

PS_IN main(in VS_IN input)
{
	PS_IN output;
	
	//positoin=============================
	// ワールド、ビュー、プロジェクション行列を掛け合わせて座標変換を行う
	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);
	
	
	//color=============================//光の影響なし
	output.pos = mul(input.pos, wvp);
	output.tex = input.tex;
	output.col = input.col;
	
	//texture=============================
	// テクスチャ座標はそのまま使用
	float4 uv;
	uv.xy = input.tex;
	uv.z = 0.0f;
	uv.w = 1.0f;
	uv = mul(uv, matrixTex);
	output.tex = uv.xy;
	
	return output;
}

