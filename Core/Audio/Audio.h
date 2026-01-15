#pragma once

//==============================================================================
// Audio.h - オーディオ管理クラス
//==============================================================================

#include "Core/System/main.h"
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

//==============================================================================
// Audio クラス
// - XAudio2によるサウンド再生
// - WAVファイルの読み込みと再生
//==============================================================================
class Audio
{
public:
	Audio();
	~Audio();

	// コピー禁止
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	//--------------------------------------------------------------------------
	// 静的メンバ（マスターボイス管理）
	//--------------------------------------------------------------------------
	static bool InitMaster();
	static void UninitMaster();

	//--------------------------------------------------------------------------
	// インスタンスメンバ（個別サウンド）
	//--------------------------------------------------------------------------
	bool Load(const std::string& fileName);
	void Unload();

	void Play(bool loop = false);
	void Stop();
	void Pause();
	void Resume();

	// 音量設定（0.0〜1.0）
	void SetVolume(float volume);
	float GetVolume() const;

	// ピッチ設定（0.5〜2.0、1.0が通常）
	void SetPitch(float pitch);

	// 再生中かどうか
	bool IsPlaying() const;

private:
	static IXAudio2* s_XAudio;
	static IXAudio2MasteringVoice* s_MasteringVoice;

	IXAudio2SourceVoice* m_SourceVoice = nullptr;
	BYTE* m_SoundData = nullptr;
	DWORD m_SoundSize = 0;
	WAVEFORMATEX m_WaveFormat = {};

	bool m_IsPlaying = false;
	bool m_IsPaused = false;
	bool m_IsLoop = false;
};
