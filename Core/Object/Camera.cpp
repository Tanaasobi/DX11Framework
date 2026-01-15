//==============================================================================
// Camera.cpp - カメラコンポーネント実装
//==============================================================================

#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"
#include "../Graphics/Renderer.h"

using namespace DirectX;

//==============================================================================
// 静的メンバ変数
//==============================================================================
Camera* Camera::s_MainCamera = nullptr;

//==============================================================================
// コンストラクタ
//==============================================================================
Camera::Camera()
	: fov(45.0f)
	, nearClip(0.1f)
	, farClip(1000.0f)
	, aspectRatio(0.0f)
{
}

//==============================================================================
// 開始
//==============================================================================
void Camera::Start()
{
	// 最初のカメラをメインカメラに設定
	if (s_MainCamera == nullptr)
	{
		s_MainCamera = this;
	}
}

//==============================================================================
// 更新
//==============================================================================
void Camera::Update(float deltaTime)
{
	// メインカメラならRendererに行列をセット
	if (s_MainCamera == this)
	{
		Renderer::SetViewMatrix(GetViewMatrix());
		Renderer::SetProjectionMatrix(GetProjectionMatrix());

		// カメラ位置もセット
		Transform* transform = GetGameObject()->GetTransform();
		Renderer::SetCameraPosition(transform->position.ToXMFLOAT3());
	}
}

//==============================================================================
// ビュー行列を取得
//==============================================================================
XMMATRIX Camera::GetViewMatrix() const
{
	Transform* transform = GetGameObject()->GetTransform();

	// カメラ位置
	XMVECTOR eye = transform->position.ToXMVECTOR();

	// 前方向ベクトルからターゲット位置を計算
	Vector3 forward = transform->GetForward();
	XMVECTOR target = XMVectorAdd(eye, forward.ToXMVECTOR());

	// 上方向ベクトル
	Vector3 up = transform->GetUp();

	return XMMatrixLookAtLH(eye, target, up.ToXMVECTOR());
}

//==============================================================================
// プロジェクション行列を取得
//==============================================================================
XMMATRIX Camera::GetProjectionMatrix() const
{
	float aspect = aspectRatio;

	// アスペクト比が0なら画面サイズから自動計算
	if (aspect <= 0.0f)
	{
		aspect = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
	}

	return XMMatrixPerspectiveFovLH(
		XMConvertToRadians(fov),
		aspect,
		nearClip,
		farClip
	);
}

//==============================================================================
// ターゲットを見る
//==============================================================================
void Camera::LookAt(const Vector3& target)
{
	Transform* transform = GetGameObject()->GetTransform();
	transform->LookAt(target);
}

//==============================================================================
// スクリーン座標からワールドレイを取得
//==============================================================================
void Camera::ScreenToWorldRay(int screenX, int screenY, Vector3& rayOrigin, Vector3& rayDirection) const
{
	Transform* transform = GetGameObject()->GetTransform();

	// スクリーン座標を正規化デバイス座標に変換（-1〜1）
	float ndcX = (2.0f * screenX / SCREEN_WIDTH) - 1.0f;
	float ndcY = 1.0f - (2.0f * screenY / SCREEN_HEIGHT);

	// 逆プロジェクション行列
	XMMATRIX invProj = XMMatrixInverse(nullptr, GetProjectionMatrix());

	// 逆ビュー行列
	XMMATRIX invView = XMMatrixInverse(nullptr, GetViewMatrix());

	// ニアプレーンとファープレーンの点
	XMVECTOR nearPoint = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
	XMVECTOR farPoint = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

	// ビュー空間に変換
	nearPoint = XMVector4Transform(nearPoint, invProj);
	farPoint = XMVector4Transform(farPoint, invProj);

	// wで除算
	nearPoint = XMVectorDivide(nearPoint, XMVectorSplatW(nearPoint));
	farPoint = XMVectorDivide(farPoint, XMVectorSplatW(farPoint));

	// ワールド空間に変換
	nearPoint = XMVector4Transform(nearPoint, invView);
	farPoint = XMVector4Transform(farPoint, invView);

	// レイの原点と方向を設定
	XMFLOAT3 origin, far3;
	XMStoreFloat3(&origin, nearPoint);
	XMStoreFloat3(&far3, farPoint);

	rayOrigin = Vector3(origin.x, origin.y, origin.z);

	Vector3 farVec(far3.x, far3.y, far3.z);
	rayDirection = (farVec - rayOrigin).Normalized();
}

//==============================================================================
// View/Projection行列をRendererにセット
//==============================================================================
void Camera::SetMatrices()
{
	using namespace DirectX;

	Transform* transform = GetGameObject()->GetTransform();

	// View行列
	XMVECTOR eye = transform->position.ToXMVECTOR();
	XMVECTOR forward = transform->GetForward().ToXMVECTOR();
	XMVECTOR up = transform->GetUp().ToXMVECTOR();
	XMVECTOR at = eye + forward;

	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	Renderer::SetViewMatrix(view);

	// Projection行列
	float aspect = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(fov),
		aspect,
		nearClip,
		farClip
	);
	Renderer::SetProjectionMatrix(proj);

	// カメラ位置をセット
	Renderer::SetCameraPosition(transform->position.ToXMFLOAT3());
}
