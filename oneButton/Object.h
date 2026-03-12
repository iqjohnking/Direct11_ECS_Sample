#pragma once
#include "Camera.h"
#include "Shader.h"

class Object {
protected:

	bool isActive = true;            // 活性状態かどうか
	int m_DrawOrder = 0;

	// SRT情報（姿勢情報）
	DirectX::SimpleMath::Vector3 m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	// 描画の為の情報（見た目に関わる部分）
	Shader m_Shader; // シェーダー

public:
	virtual ~Object() {} // 仮想デストラクタ（※派生クラスのリソース解放のために必要）

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw(Camera* cam) = 0;
	virtual void Uninit() = 0;

	// 位置の取得
	DirectX::SimpleMath::Vector3 GetPosition() const { return m_Position; }
	void SetPosition(DirectX::SimpleMath::Vector3 position) { m_Position = position; }

	DirectX::SimpleMath::Vector3 GetRotation() const { return m_Rotation; }
	void SetRotation(DirectX::SimpleMath::Vector3 rotation) { m_Rotation = rotation; }

	DirectX::SimpleMath::Vector3 GetScale() const { return m_Scale; }
	void SetScale(DirectX::SimpleMath::Vector3 scale) { m_Scale = scale; }

	void SetActive(bool act) { isActive = act; }
	bool IsActive() const { return isActive; }
};
