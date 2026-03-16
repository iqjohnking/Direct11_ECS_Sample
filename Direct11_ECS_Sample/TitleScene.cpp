#include "TitleScene.h"
#include "Game.h"
#include "Input.h"
#include "Texture2D.h"
#include "GolfBall.h"

using namespace std;

// コンストラクタ
TitleScene::TitleScene()
{
	Init();
}

// デストラクタ
TitleScene::~TitleScene()
{
	Uninit();
}

// 初期化
void TitleScene::Init()
{	
	//背景画像オブジェクト生成
	Texture2D* bg = Game::GetInstance()->AddObject<Texture2D>();
	bg->SetTexture("assets/texture/background1.png");
	bg->SetPosition(0.0f, 0.0f, 0.0f);
	bg->SetRotation(0.0f, 0.0f, 0.0f);
	//bg->SetScale(1600.0f, 900.0f, 0.0f);
	bg->SetScale(1920.0f, 1080.0f, 0.0f);
	m_MySceneObjects.emplace_back(bg);

	GolfBall* gb = Game::GetInstance()->AddObject<GolfBall>();
	m_MySceneObjects.emplace_back(gb);

}

void TitleScene::DrawImGui()
{
	const bool titleOpen = ImGui::Begin("Title Scene");
	if (titleOpen)
	{
		ImGui::Text("testObjs Vector3");
		vector<GolfBall*> testObjs = Game::GetInstance()->GetObjects<GolfBall>();
		if (!testObjs.empty())
		{
			const auto position = testObjs[0]->GetPosition();
			float pos[3] = { position.x, position.y, position.z };
			if (ImGui::DragFloat3("Model Position", pos, 0.1f))
			{
				testObjs[0]->SetPosition(DirectX::SimpleMath::Vector3(pos[0], pos[1], pos[2]));
			}
		}
	}
	ImGui::End();

	// 他ウィンドウも同じパターン
	const bool debugOpen = ImGui::Begin("Debug");
	if (debugOpen)
	{
		ImGui::Text("Enterキーで Stage1 へ");
	}
	ImGui::End();
}

// 更新
void TitleScene::Update()
{
	// エンターキーを押してステージ1へ
	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Game::GetInstance()->ChangeScene(SceneName::STAGE1);
	}
}

// 終了処理
void TitleScene::Uninit()
{
	// このシーンのオブジェクトを削除する
	for (auto& o : m_MySceneObjects) {
		Game::GetInstance()->DeleteObject(o);
	}
	m_MySceneObjects.clear();
}
