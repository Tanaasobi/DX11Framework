#pragma once

//==============================================================================
// Field.h - フィールド
//==============================================================================

#include "Core/Object/GameObject.h"
#include "Core/Graphics/Shader/IShader.h"

//==============================================================================
// フィールド定数
//==============================================================================
namespace FieldBounds
{
	constexpr float WIDTH = 40.0f;  
	constexpr float HEIGHT = 24.0f;   
	constexpr float LEFT = -WIDTH * 0.5f;
	constexpr float RIGHT = WIDTH * 0.5f;
	constexpr float TOP = -HEIGHT * 0.5f;
	constexpr float BOTTOM = HEIGHT * 0.5f;
}

//==============================================================================
// Field クラス
//==============================================================================
class Field : public GameObject
{
public:
	Field();
	virtual ~Field();

	void Init(IShader* shader);
};
