#pragma once

#include <xaudio2.h>

// サウンドファイル
typedef enum
{
	SOUND_LABEL_BGM000 = 0,	// サンプルBGM
	SOUND_LABEL_SE000,		// サンプルSE
	SOUND_LABEL_SE001,		// サンプルSE
	SOUND_LABEL_SE002,		// サンプルSE
	SOUND_LABEL_SE003,		// サンプルSE
	SOUND_LABEL_SE004,		// サンプルSE
	SOUND_LABEL_SE005,		// サンプルSE
	SOUND_LABEL_SE006,		// サンプルSE
	SOUND_LABEL_SE007,		// サンプルSE
	SOUND_LABEL_SE008,		// サンプルSE

	SOUND_LABEL_MAX,
} SOUND_LABEL;

class Sound {
private:
	// パラメータ構造体
	typedef struct
	{
		LPCSTR filename;	// 音声ファイルまでのパスを設定
		bool bLoop;			// trueでループ。通常BGMはture、SEはfalse。
		float volume; //音量
	} PARAM;

	//（ループさせるのでtrue設定）
	PARAM m_param[SOUND_LABEL_MAX] =
	{
		{"assets/sound/bgm_1.wav"		, true , 0.08f},						// SOUND_LABEL_BGM00
		{"assets/sound/bilibili.wav"	, true , 0.70f},							// SOUND_LABEL_SE000
		{"assets/sound/biiiii.wav"		, true , 0.70f},							// SOUND_LABEL_SE001
		{"assets/sound/Heartbeat04-4(Slow-Reverb-Short).wav"	, true , 1.20f},	// SOUND_LABEL_SE002
		{"assets/sound/Heartbeat04-5(Mid-Reverb-Short).wav"		, true , 1.20f},	// SOUND_LABEL_SE003
		{"assets/sound/Heartbeat04-6(Fast-Reverb-Short).wav"	, true , 1.20f},	// SOUND_LABEL_SE004
		{"assets/sound/step.wav"		, true , 1.20f},							// SOUND_LABEL_SE005
		{"assets/sound/attack.wav"		, false , 1.50f},						// SOUND_LABEL_SE006
		{"assets/sound/dead.wav"		, false , 2.00f},						// SOUND_LABEL_SE007
		{"assets/sound/restart.wav"		, false , 1.00f},						// SOUND_LABEL_SE008
	};

	IXAudio2* m_pXAudio2 = NULL;
	IXAudio2MasteringVoice* m_pMasteringVoice = NULL;
	IXAudio2SourceVoice* m_pSourceVoice[SOUND_LABEL_MAX];
	WAVEFORMATEXTENSIBLE m_wfx[SOUND_LABEL_MAX]; // WAVフォーマット
	XAUDIO2_BUFFER m_buffer[SOUND_LABEL_MAX];
	BYTE* m_DataBuffer[SOUND_LABEL_MAX];

	HRESULT FindChunk(HANDLE, DWORD, DWORD&, DWORD&);
	HRESULT ReadChunkData(HANDLE, void*, DWORD, DWORD);

public:
	// ゲームループ開始前に呼び出すサウンドの初期化処理
	HRESULT Init(void);

	// ゲームループ終了後に呼び出すサウンドの解放処理
	void Uninit(void);

	// 引数で指定したサウンドを再生する
	void Play(SOUND_LABEL label);

	// 引数で指定したサウンドを停止する
	void Stop(SOUND_LABEL label);

	// 引数で指定したサウンドの再生を再開する
	void Resume(SOUND_LABEL label);

	// 引数で指定したサウンドの再生速度を変更する
	bool SetSpeed(SOUND_LABEL label, float ratio);

	// 引数で指定したサウンドの再生が完了したか
	bool IsFinished(SOUND_LABEL label) const;

};