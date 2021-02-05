// dx12_box_tutorial.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Main.h"
#include <string>
#include "Engine.h"
#include "d3dUtility.h"
#include "CameraManager.h"
#include <iostream>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int initialWidth{ 900 };
	int initialHeight{ 600 };
	std::wstring initialTitle = L"DirectX12 Box Tutorial";

	auto windowManager = WindowManager(
		hInstance, nCmdShow, initialHeight, initialWidth, initialTitle);
	auto cameraManager = CameraManager();
	FPSCamera camera = cameraManager.GetFPSCamera();
	auto inputManager = InputManager(camera);

	try {
		auto engine = Engine(windowManager, camera, inputManager);

		engine.Initialize();

		MSG msg{ 0 };
		while (msg.message != WM_QUIT)
		{
			// Handle window messages such as mouse and keyboard activities
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				engine.Update();
				engine.Render();
			}
		}
		engine.Destroy();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
	}

	return 0;
}
