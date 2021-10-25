#include "stdafx.h"
#include "MainGame.h"
#include "GeneralStructs.h"
#include "SceneManager.h"
#include "PhysxProxy.h"
#include "DebugRenderer.h"
#include "MainMenu.h"

MainGame::MainGame(void)
{}

MainGame::~MainGame(void)
{}

//Game is preparing
void MainGame::OnGamePreparing(GameSettings& gameSettings)
{
	UNREFERENCED_PARAMETER(gameSettings);
}

void MainGame::Initialize()
{
	ShowCursor(false);

	SceneManager::GetInstance()->AddGameScene(new MainMenu());
	SceneManager::GetInstance()->SetActiveGameScene(L"Main menu");
}

LRESULT MainGame::WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_KEYUP:
	{
		if ((lParam & 0x80000000) != 0x80000000)
			return -1;

		//NextScene
		if (wParam == VK_F3)
		{
			SceneManager::GetInstance()->NextScene();
			return 0;
		}
		//PreviousScene
		else if (wParam == VK_F2)
		{
			SceneManager::GetInstance()->PreviousScene();
			return 0;
		}
		else if (wParam == VK_F4)
		{
			DebugRenderer::ToggleDebugRenderer();
			return 0;
		}
		else if (wParam == VK_F6)
		{
			auto activeScene = SceneManager::GetInstance()->GetActiveScene();
			activeScene->GetPhysxProxy()->NextPhysXFrame();
		}
	}
	}
	return -1;
}
