#pragma once

//==============================================================================
// Camera.h - カメラコンポーネント
//==============================================================================

#include "Component.h"
#include "../Math/Vector3.h"

//==============================================================================
// Camera クラス
// - ビュー行列・プロジェクション行列を生成
// - メインカメラの管理
//==============================================================================
class Camera : public Component
{
public:
	Camera();
	virtual ~Camera() = default;

	void Start() override;
	void Update(float deltaTime) override;

	//--------------------------------------------------------------------------
	// 行列取得
	//--------------------------------------------------------------------------
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;

	//--------------------------------------------------------------------------
	// カメラパラメータ
	//--------------------------------------------------------------------------
	float fov = 45.0f;      // 視野角（度）
	float nearClip = 0.1f;       // ニアクリップ
	float farClip = 1000.0f;    // ファークリップ
	float aspectRatio = 0.0f;       // アスペクト比（0で自動計算）

	//--------------------------------------------------------------------------
	// メインカメラ
	//--------------------------------------------------------------------------
	static Camera* GetMain() { return s_MainCamera; }
	void SetAsMain() { s_MainCamera = this; }

	//--------------------------------------------------------------------------
	// カメラ操作
	//--------------------------------------------------------------------------
	// ターゲットを見る
	void LookAt(const Vector3& target);

	// スクリーン座標からワールド座標へのレイを取得
	void ScreenToWorldRay(int screenX, int screenY, Vector3& rayOrigin, Vector3& rayDirection) const;

	// View/Projection行列をRendererにセット
	void SetMatrices();

private:
	static Camera* s_MainCamera;
};
