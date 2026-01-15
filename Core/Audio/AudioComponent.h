#pragma once

//==============================================================================
// AudioComponent.h - オーディオコンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Audio.h"

//==============================================================================
// AudioComponent クラス
// - GameObjectにアタッチ可能なオーディオ再生コンポーネント
//==============================================================================
class AudioComponent : public Component
{
public:
	AudioComponent();
	virtual ~AudioComponent();

	// サウンド読み込み
	bool Load(const std::string& fileName);

	// 再生制御
	void Play(bool loop = false);
	void Stop();
	void Pause();
	void Resume();

	// 設定
	void SetVolume(float volume);
	float GetVolume() const;
	void SetPitch(float pitch);

	// 状態
	bool IsPlaying() const;

	// 開始時に自動再生するか
	bool playOnStart = false;
	bool loop = false;

protected:
	void Start() override;

private:
	Audio m_Audio;
	std::string m_FileName;
	bool m_Loaded = false;
};
