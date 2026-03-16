#include "Renderer.h"
#include "Camera.h"
#include "Application.h"

using namespace DirectX::SimpleMath;

//=======================================
//初期化処理
//=======================================
void Camera::Init()
{
	m_Position = Vector3(0.0f, 10.0f, -50.0f);
	m_Target = Vector3(0.0f, 0.0f, 0.0f);
}


//=======================================
//更新処理
//=======================================
void Camera::Update()
{

}

//=======================================
//描画処理
//=======================================

void Camera::SetCamera(int mode)
{
	//3D
	if (mode == 0)
	{
		// ビュー変換後列作成
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(m_Position, m_Target, up); //左手系//この関数は自動的にカメラをm_Target向けに

		Renderer::SetViewMatrix(&m_ViewMatrix);

		//プロジェクション行列の生成
		constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角 // ConvertToRadians() は角度をラジアンに変換する関数です。

		float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
		float nearPlane = 1.0f;			 // ニアクリップ
		float farPlane = 1000.0f;		 // ファークリップ

		Matrix projectionMatrix;
		projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			fieldOfView,
			aspectRatio,
			nearPlane, farPlane);	//左手系

		Renderer::SetProjectionMatrix(&projectionMatrix);
	}
	else if (mode == 1)//2D
	{
		// ??
		constexpr float virtualWidth = 1920.0f;
		constexpr float virtualHeight = 1080.0f;

		// ビュー変換後列作成
		Vector3 pos = Vector3(0.0f, 0.0f, -10.0f);
		Vector3 tgt = Vector3(0.0f, 0.0f, 1.0f);
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		m_ViewMatrix = DirectX::XMMatrixLookAtLH(pos, tgt, up);

		Renderer::SetViewMatrix(&m_ViewMatrix);

		//プロジェクション行列の生成
		float nearPlane = 1.0f;			 // ニアクリップ
		float farPlane = 1000.0f;		 // ファークリップ

		Matrix projectionMatrix;

		projectionMatrix = DirectX::XMMatrixOrthographicLH(
							virtualWidth,
							virtualHeight,
							nearPlane, farPlane);	//左手系// 2D 用の正射影行列を作成する

		projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);// シェーダーが正しく解釈できるように行列を転置する

		Renderer::SetProjectionMatrix(&projectionMatrix);
	}
}

//void Camera::SetCamera()
//{
//	// ビュー変換後列作成
//	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
//	m_ViewMatrix = DirectX::XMMatrixLookAtLH(m_Position, m_Target, up); //左手系
//
//	Renderer::SetViewMatrix(&m_ViewMatrix);
//
//	//プロジェクション行列の生成
//	constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角
//	
//	float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
//	float nearPlane = 1.0f;       // ニアクリップ
//	float farPlane = 1000.0f;      // ファークリップ
//
//	//プロジェクション行列の生成
//	Matrix projectionMatrix;
//	projectionMatrix =  DirectX::XMMatrixPerspectiveFovLH( fieldOfView,  aspectRatio,  nearPlane,  farPlane);	//左手系
//
//	Renderer::SetProjectionMatrix(&projectionMatrix);
//}


//=======================================
//終了処理
//=======================================
void Camera::Uninit()
{

}

