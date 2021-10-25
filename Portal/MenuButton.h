#pragma once
#include <GameObject.h>

class SpriteComponent;

class MenuButton : public GameObject
{
public:
	MenuButton(const std::wstring& path, const std::wstring& pathHovered);
	~MenuButton() = default;

	MenuButton(const MenuButton& other) = delete;
	MenuButton(MenuButton&& other) noexcept = delete;
	MenuButton& operator=(const MenuButton& other) = delete;
	MenuButton& operator=(MenuButton&& other) noexcept = delete;

	void SetIsSelected(bool value);
	void SetOnButtonClicked(std::function<void()> buttonClicked);

	void ButtonClicked();
protected:
	virtual void Initialize(const GameContext&);
private:
	//datamembers
	SpriteComponent* m_pButton, * m_pHoveredButton;
	std::function<void()> m_ButtonClicked;
};

