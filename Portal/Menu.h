#pragma once
#include <GameScene.h>

class MenuButton;

class Menu : public GameScene
{
public:
	//rule of 5
	Menu(const std::wstring& sceneName);
	virtual ~Menu() = default;

	Menu(const Menu& other) = delete;
	Menu(Menu&& other) noexcept = delete;
	Menu& operator=(const Menu& other) = delete;
	Menu& operator=(Menu&& other) noexcept = delete;

	//functions
	void AddButton(MenuButton* pButton);
protected:
	virtual void Initialize() override;
	virtual void Draw()  override {};
	virtual void Update() override;

private:
	//datamembers
	unsigned int m_MenuSelectionIterator;
	std::vector<MenuButton*> m_pMenuButtons;
	FMOD::Sound* m_pSelectSound, * m_pNextSound;
};

