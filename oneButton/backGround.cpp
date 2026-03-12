#include "backGround.h"
#include "Game.h"
#include "input.h"

using namespace std;
using namespace DirectX::SimpleMath;

void backGround::Init()
{
	m_Texture2D.Init();
	m_Texture2D.SetTexture("assets/texture/background1.png");
	m_Texture2D.SetPosition(0.0f, 0.0f, 0.0f);
	m_Texture2D.SetRotation(0.0f, 0.0f, 0.0f);
	m_Texture2D.SetScale(1920.0f, 1080.0f, 2.0f);

	m_Texture2D.SetSpriteSheet(2, 2);
	m_Texture2D.AddAnimClip("open", 0, 3, 5);
	m_Texture2D.PlayAnim("open");

}

void backGround::Update()
{
	m_Texture2D.Update();
	//Control();
}

void backGround::Draw(Camera* cam)
{
	//m_Texture2D.SetScale(m_Radius * 2, m_Radius * 2, 0);
	//m_Texture2D.SetPosition(GetPosition());
	m_Texture2D.Draw(cam);
}
void backGround::Uninit()
{
	m_Texture2D.Uninit();
}

void backGround::Control()
{
	
}
