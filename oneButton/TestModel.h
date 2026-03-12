#pragma once

#include "Camera.h"
#include "Object.h"
//#include "Shader.h" Object.hの中があります
#include "Texture.h"
#include "MeshRenderer.h"
#include "StaticMesh.h"
#include "utility.h"
#include "Material.h"

class TestModel :public Object
{
private:
	// SRT情報（姿勢情報）
	//DirectX::SimpleMath::Vector3 m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	//DirectX::SimpleMath::Vector3 m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	//DirectX::SimpleMath::Vector3 m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// 描画の為の情報（メッシュに関わる情報）
	MeshRenderer m_MeshRenderer; // 頂点バッファ・インデックスバッファ・インデックス数

	// 描画の為の情報（見た目に関わる部分）
	//Shader m_Shader; // シェーダー
	std::vector<std::unique_ptr<Material>> m_Materiales;
	std::vector<SUBSET> m_subsets;
	std::vector<std::unique_ptr<Texture>> m_Textures; // テクスチャ

public:
	void Init();
	void Update();
	void Draw(Camera* cam);
	void Uninit();

};
