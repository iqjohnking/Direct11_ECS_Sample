#pragma once
#include <iostream>
#include <array>
#include <algorithm>
#include "sound.h"

#include "TitleScene.h"
#include "Stage1Scene.h"
#include "ResultScene.h"

enum class SceneName {
	TITLE,
	STAGE1,
	RESULT
};


class Game
{
private:
	// ゲームクラスのインスタンス
	static Game* m_Instance; 
	// シーン
	Scene* m_Scene; 
	// カメラ
	Camera  m_Camera;
	//サウンド
	static Sound m_Sound;

	// オブジェクトのリスト
	std::vector<std::unique_ptr<Object>> m_Objects; // オブジェクトのリスト

	// imgui用の変数
	bool isShowImgui = false; // ImGuiの表示フラグ

public:
	Game(); // コンストラクタ
	~Game(); // デストラクタ

	static void Init(); // 初期化
	static void Update(); // 更新
	static void Draw(); // 描画
	static void Uninit(); // 終了処理

	static Game* GetInstance(); // インスタンスの取得
	void ChangeScene(SceneName sceneName); // シーンの切り替え
	void DeleteObject(Object* obj); // オブジェクトの削除
	void DeleteAllObjects(); // 全てのオブジェクトの削除

	//サウンドを取得
	static Sound* GetSound() { return &m_Sound; }
	
	// オブジェクトの追加
	template <typename T> T* AddObject()
	{
		T* pt = new T();
		m_Instance->m_Objects.emplace_back(pt);
		pt->Init();
		return pt;
	}
	// オブジェクトの取得（型指定）
	template <typename T> std::vector<T*> GetObjects()
	{
		std::vector<T*> result;
		for (const auto& obj : m_Instance->m_Objects)
		{
			if (T* derivedObj = dynamic_cast<T*>(obj.get()))
			{
				result.emplace_back(derivedObj); // derivedObjは派生オブジェクトの意味です
			}
		}
		return result;
	}
};
