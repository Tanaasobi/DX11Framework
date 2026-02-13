#pragma once
#include "Core/Scene/Scene.h"
#include "Core/Graphics/UI/Font.h"

class ResultScene : public Scene
{
public:
	ResultScene() : Scene("ResultScene") {}
	~ResultScene() { if (m_Font) { m_Font->Uninit(); delete m_Font; } }

	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	Font* m_Font = nullptr;
};
