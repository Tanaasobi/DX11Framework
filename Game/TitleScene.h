#pragma once
#include "Core/Scene/Scene.h"
#include "Core/Graphics/UI/Font.h"

class TitleScene : public Scene
{
public:
	TitleScene() : Scene("TitleScene") {}
	~TitleScene() { if (m_Font) { m_Font->Uninit(); delete m_Font; } }

	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	Font* m_Font = nullptr;
};
