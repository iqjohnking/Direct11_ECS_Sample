#pragma once

#include	<SimpleMath.h>

//-----------------------------------------------------------------------------
//CameraƒNƒ‰ƒX
//-----------------------------------------------------------------------------
class Camera {
private:
	DirectX::SimpleMath::Vector3	m_Position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Rotation = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3	m_Scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);

	DirectX::SimpleMath::Vector3	m_Target{};
	DirectX::SimpleMath::Matrix		m_ViewMatrix{};

	static constexpr float	TWO_PI = 6.283185307f;
	static constexpr float		PI = 3.1415926535f;

public:
	void Init();
	void Update();
	void SetCamera(int mode = 0);
	void Uninit();

	DirectX::SimpleMath::Vector3 GetTarget();
	void SetTarget(DirectX::SimpleMath::Vector3 target);
};