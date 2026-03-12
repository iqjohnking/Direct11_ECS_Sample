#include "Game.h"
#include "Renderer.h"
#include "input.h"
#include "Application.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

Game* Game::m_Instance = nullptr; // ゲームクラスのインスタンス
Sound Game::m_Sound;

// コンストラクタ
Game::Game()
{
	m_Scene = nullptr;
}

// デストラクタ
Game::~Game()
{
	delete m_Scene;
	DeleteAllObjects();
}

// 初期化
void Game::Init()
{
	// ゲームクラスのインスタンスを保存
	m_Instance = new Game; 
	// 描画初期化
	Renderer::Init();
	// 入力初期化
	Input::Create();

	//サウンド初期化
	m_Instance->m_Sound.Init();
	// カメラ初期化
	m_Instance->m_Camera.Init();
	// シーン初期化
	m_Instance->m_Scene = new TitleScene();

	// ImGui初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Application::GetWindow());
	ImGui_ImplDX11_Init(Renderer::GetDevice(), Renderer::GetDeviceContext());

}

// 更新
void Game::Update()
{
	Input::Update();
	// シーン更新
	m_Instance->m_Scene->Update();
	// カメラ更新
	m_Instance->m_Camera.Update();
	// オブジェクト更新
	for (auto& obj : m_Instance->m_Objects)
	{
		obj->Update();
	}
	if (Input::GetKeyTrigger('M'))
	{
		m_Instance->isShowImgui = !m_Instance->isShowImgui;
	}

}

// 描画
void Game::Draw()
{
	// 描画前処理
	Renderer::DrawStart();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// オブジェクト描画
	for (auto& obj : m_Instance->m_Objects)
	{
		obj->Draw(&m_Instance->m_Camera);
	}
	//// 1) m_Objects から生描画リストを作成
	//std::vector<Object*> drawList;
	//drawList.reserve(m_Instance->m_Objects.size());
	//for (auto& up : m_Instance->m_Objects) {
	//	drawList.push_back(up.get());
	//}
	//// 2) 手動描画順で安定ソート（同値は生成順を保持）// 小さい順に描画される（背面→前面）
	//std::stable_sort(drawList.begin(), drawList.end(),
	//	[](const Object* a, const Object* b) {
	//		return a->GetDrawOrder() < b->GetDrawOrder();
	//	});
	//// 3) 並び替えた順で描画
	//for (auto* o : drawList) {
	//	o->Draw(&m_Instance->m_Camera);
	//}

	// ImGui描画
	if (m_Instance->m_Scene != nullptr && m_Instance->isShowImgui)
	{
		m_Instance->m_Scene->DrawImGui();   // ★ シーン毎のUI
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	

	// 描画後処理
	Renderer::DrawEnd();
}

// 終了処理
void Game::Uninit()
{
	// カメラ終了処理
	m_Instance->m_Camera.Uninit();
	// オブジェクト終了処理
	for (auto& obj : m_Instance->m_Objects)
	{
		obj->Uninit();
	}
	// 入力終了処理
	Input::Release();

	// ImGui終了処理
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// サウンド終了処理
	m_Sound.Uninit();

	// 描画終了処理
	Renderer::Uninit();

	// インスタンスの解放
	delete m_Instance;
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝


Game* Game::GetInstance()
{
	return m_Instance;
}

void Game::ChangeScene(SceneName sceneName)
{
	// 読み込み済みシーンの削除
	int score = 0;
	bool isWin = false;
	// シーンが存在する場合は削除
	if (m_Instance->m_Scene != nullptr) {
		// ステージ1シーンからリザルトシーンへ移行する場合、スコアを取得しておく
		//if (Stage1Scene* stage1Scene = dynamic_cast<Stage1Scene*>(m_Instance->m_Scene))
		//{
		//	score = stage1Scene->GetScore();
		//	isWin = stage1Scene->GetIsWin();
		//}
		delete m_Instance->m_Scene;
		m_Instance->m_Scene = nullptr;
	}
	switch (sceneName) {
	case SceneName::TITLE:
		m_Instance->m_Scene = new TitleScene();
		break;
	case SceneName::STAGE1:
		m_Instance->m_Scene = new Stage1Scene();
		break;
	case SceneName::RESULT:
		m_Instance->m_Scene = new ResultScene();
		//dynamic_cast<ResultScene*>(m_Instance->m_Scene)->SetScore(score, isWin);
		break;
	default:
		break;
	}
}
void Game::DeleteObject(Object* ptr)
{
	if (ptr == nullptr) {
		return;
	}
	ptr->Uninit();
	//要素の削除
	erase_if(m_Instance->m_Objects,
		[ptr](const std::unique_ptr<Object>& element) {
			return element.get() == ptr;
		});
	m_Instance->m_Objects.shrink_to_fit();
}
void Game::DeleteAllObjects()
{
	for (auto& o : m_Instance->m_Objects) {
		o->Uninit();
	}
	m_Instance->m_Objects.clear();
	m_Instance->m_Objects.shrink_to_fit();
}