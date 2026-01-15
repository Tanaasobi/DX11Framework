//==============================================================================
// Audio.cpp - オーディオ管理クラス実装
//==============================================================================

#include "Audio.h"
#include "Core/System/Logger.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

//==============================================================================
// 静的メンバ変数
//==============================================================================
IXAudio2* Audio::s_XAudio = nullptr;
IXAudio2MasteringVoice* Audio::s_MasteringVoice = nullptr;

//==============================================================================
// コンストラクタ
//==============================================================================
Audio::Audio()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Audio::~Audio()
{
	Unload();
}

//==============================================================================
// マスターボイス初期化
//==============================================================================
bool Audio::InitMaster()
{
	// COM初期化
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
	{
		Logger::Error("COM initialization failed");
		return false;
	}

	// XAudio2インターフェース作成
	hr = XAudio2Create(&s_XAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		Logger::Error("XAudio2Create failed");
		return false;
	}

	// マスターボイス作成
	hr = s_XAudio->CreateMasteringVoice(&s_MasteringVoice);
	if (FAILED(hr))
	{
		Logger::Error("CreateMasteringVoice failed");
		s_XAudio->Release();
		s_XAudio = nullptr;
		return false;
	}

	Logger::Info("Audio system initialized");
	return true;
}

//==============================================================================
// マスターボイス終了
//==============================================================================
void Audio::UninitMaster()
{
	if (s_MasteringVoice)
	{
		s_MasteringVoice->DestroyVoice();
		s_MasteringVoice = nullptr;
	}

	if (s_XAudio)
	{
		s_XAudio->Release();
		s_XAudio = nullptr;
	}

	CoUninitialize();

	Logger::Info("Audio system uninitialized");
}

//==============================================================================
// WAVファイル読み込み
//==============================================================================
bool Audio::Load(const std::string& fileName)
{
	// 既に読み込み済みなら解放
	Unload();

	// ファイルを開く
	HMMIO hmmio = mmioOpenA((LPSTR)fileName.c_str(), nullptr, MMIO_ALLOCBUF | MMIO_READ);
	if (!hmmio)
	{
		Logger::ErrorFormat("Failed to open audio file: %s", fileName.c_str());
		return false;
	}

	// RIFFチャンクを探す
	MMCKINFO riffChunk;
	riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, &riffChunk, nullptr, MMIO_FINDRIFF) != MMSYSERR_NOERROR)
	{
		Logger::Error("Invalid WAV file: RIFF chunk not found");
		mmioClose(hmmio, 0);
		return false;
	}

	// fmtチャンクを探す
	MMCKINFO fmtChunk;
	fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hmmio, &fmtChunk, &riffChunk, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		Logger::Error("Invalid WAV file: fmt chunk not found");
		mmioClose(hmmio, 0);
		return false;
	}

	// フォーマット情報を読み込む
	if (mmioRead(hmmio, (HPSTR)&m_WaveFormat, sizeof(m_WaveFormat)) != sizeof(m_WaveFormat))
	{
		Logger::Error("Failed to read WAV format");
		mmioClose(hmmio, 0);
		return false;
	}

	mmioAscend(hmmio, &fmtChunk, 0);

	// dataチャンクを探す
	MMCKINFO dataChunk;
	dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		Logger::Error("Invalid WAV file: data chunk not found");
		mmioClose(hmmio, 0);
		return false;
	}

	// サウンドデータを読み込む
	m_SoundSize = dataChunk.cksize;
	m_SoundData = new BYTE[m_SoundSize];

	if (mmioRead(hmmio, (HPSTR)m_SoundData, m_SoundSize) != (LONG)m_SoundSize)
	{
		Logger::Error("Failed to read WAV data");
		delete[] m_SoundData;
		m_SoundData = nullptr;
		mmioClose(hmmio, 0);
		return false;
	}

	mmioClose(hmmio, 0);

	// ソースボイス作成
	HRESULT hr = s_XAudio->CreateSourceVoice(&m_SourceVoice, &m_WaveFormat);
	if (FAILED(hr))
	{
		Logger::Error("CreateSourceVoice failed");
		delete[] m_SoundData;
		m_SoundData = nullptr;
		return false;
	}

	Logger::InfoFormat("Audio loaded: %s", fileName.c_str());
	return true;
}

//==============================================================================
// 解放
//==============================================================================
void Audio::Unload()
{
	Stop();

	if (m_SourceVoice)
	{
		m_SourceVoice->DestroyVoice();
		m_SourceVoice = nullptr;
	}

	if (m_SoundData)
	{
		delete[] m_SoundData;
		m_SoundData = nullptr;
	}

	m_SoundSize = 0;
}

//==============================================================================
// 再生
//==============================================================================
void Audio::Play(bool loop)
{
	if (!m_SourceVoice || !m_SoundData) return;

	// 再生中なら停止
	Stop();

	m_IsLoop = loop;

	// バッファ設定
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = m_SoundSize;
	buffer.pAudioData = m_SoundData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	if (loop)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// バッファを送信して再生開始
	m_SourceVoice->SubmitSourceBuffer(&buffer);
	m_SourceVoice->Start();

	m_IsPlaying = true;
	m_IsPaused = false;
}

//==============================================================================
// 停止
//==============================================================================
void Audio::Stop()
{
	if (!m_SourceVoice) return;

	m_SourceVoice->Stop();
	m_SourceVoice->FlushSourceBuffers();

	m_IsPlaying = false;
	m_IsPaused = false;
}

//==============================================================================
// 一時停止
//==============================================================================
void Audio::Pause()
{
	if (!m_SourceVoice || !m_IsPlaying || m_IsPaused) return;

	m_SourceVoice->Stop();
	m_IsPaused = true;
}

//==============================================================================
// 再開
//==============================================================================
void Audio::Resume()
{
	if (!m_SourceVoice || !m_IsPaused) return;

	m_SourceVoice->Start();
	m_IsPaused = false;
}

//==============================================================================
// 音量設定
//==============================================================================
void Audio::SetVolume(float volume)
{
	if (!m_SourceVoice) return;

	// 0.0〜1.0にクランプ
	volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
	m_SourceVoice->SetVolume(volume);
}

float Audio::GetVolume() const
{
	if (!m_SourceVoice) return 0.0f;

	float volume;
	m_SourceVoice->GetVolume(&volume);
	return volume;
}

//==============================================================================
// ピッチ設定
//==============================================================================
void Audio::SetPitch(float pitch)
{
	if (!m_SourceVoice) return;

	// 0.5〜2.0にクランプ
	pitch = (pitch < 0.5f) ? 0.5f : (pitch > 2.0f) ? 2.0f : pitch;
	m_SourceVoice->SetFrequencyRatio(pitch);
}

//==============================================================================
// 再生中かどうか
//==============================================================================
bool Audio::IsPlaying() const
{
	if (!m_SourceVoice) return false;

	XAUDIO2_VOICE_STATE state;
	m_SourceVoice->GetState(&state);

	return (state.BuffersQueued > 0) && !m_IsPaused;
}
