#include "stdafx.h"
#include "MenuButton.h"
#include "SpriteComponent.h"

MenuButton::MenuButton(const std::wstring& path, const std::wstring& pathHovered)
	:m_pButton{ new SpriteComponent(path, DirectX::XMFLOAT2(0.f, 0.f), DirectX::XMFLOAT4(1, 1, 1, 1.f)) },
	m_pHoveredButton{ new SpriteComponent(pathHovered, DirectX::XMFLOAT2(0.f, 0.f), DirectX::XMFLOAT4(1, 1, 1, 1.f)) },
	m_ButtonClicked{ nullptr }
{}

void MenuButton::SetIsSelected(bool value)
{
	m_pButton->SetIsActive(!value);
	m_pHoveredButton->SetIsActive(value);
}

void MenuButton::SetOnButtonClicked(std::function<void()> buttonClicked)
{
	m_ButtonClicked = buttonClicked;
}

void MenuButton::ButtonClicked()
{
	if (m_ButtonClicked != nullptr) m_ButtonClicked();
}

void MenuButton::Initialize(const GameContext&)
{
	AddComponent(m_pButton);
	AddComponent(m_pHoveredButton);
	m_pButton->SetIsActive(true);
	m_pHoveredButton->SetIsActive(false);
}

