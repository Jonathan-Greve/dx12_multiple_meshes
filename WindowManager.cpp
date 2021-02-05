#include "WindowManager.h"
#include "Resource.h"
#include <wtypes.h>
#include <string>
#include "EventManager.h"
#include <windowsx.h>

static bool isResizing;

WindowManager::WindowManager(HINSTANCE hInstance, int cmdShow, int height = 500, int width = 500, std::wstring title = L"Win32 app")
{
	m_hInst = hInstance;
	m_cmdShow = cmdShow;
	m_height = height;
	m_width = width;
	m_title = title;
}

BOOL WindowManager::CreateMainWindow()
{
	RegisterWindow();
	if (!InitializeWindow()) return FALSE;
	if (!ShowAppWindow()) return FALSE;
	if (!UpdateAppWindow()) return FALSE;

	EventManager().AddListener<WindowResizedEvent>("CreateMainWindowResizedEvent",
		[&](WindowResizedEvent& event)
		{
			UpdateHeight(event.height);
			UpdateWidth(event.width);
		});

	return TRUE;
}

int WindowManager::GetHeight()
{
	return m_height;
}

int WindowManager::GetWidth()
{
	return m_width;
}

const std::wstring& WindowManager::GetTitle()
{
	return m_title;
}

const HWND& WindowManager::GetMainWindow()
{
	return m_hMainWnd;
}

void WindowManager::UpdateHeight(int newHeight)
{
	m_height = newHeight;
}

void WindowManager::UpdateWidth(int newWidth)
{
	m_width = newWidth;
}

void WindowManager::UpdateDimensions(int newHeight, int newWidth)
{
	m_height = max(min_height, newHeight);
	m_width = max(min_width, newWidth);

}

LRESULT CALLBACK WindowManager::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowManager* windowManager = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		// Save the Windowmanager* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;
	case WM_SIZE:
	{
		int newWidth = LOWORD(lParam);
		int newHeight = HIWORD(lParam);
		windowManager->UpdateDimensions(newHeight, newWidth);
		auto resizedEvent = WindowResizedEvent(newWidth, newHeight);
		// Save the new window dimensions.
		if (wParam == SIZE_MINIMIZED)
		{
			EventManager().NotifyListeners<WindowResizedEvent>(resizedEvent);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			EventManager().NotifyListeners<WindowResizedEvent>(resizedEvent);
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (windowManager->isRezising) {}
			else {
				EventManager().NotifyListeners<WindowResizedEvent>(resizedEvent);
			}
		}
		return 0;
	}

	//// When the resizing begins.
	case WM_ENTERSIZEMOVE:
		windowManager->isRezising = true;
		return 0;

		//// When the resizing is complete.
	case WM_EXITSIZEMOVE:
	{
		windowManager->isRezising = false;
		auto resizedEvent = WindowResizedEvent(windowManager->m_width, windowManager->m_height);
		EventManager().NotifyListeners<WindowResizedEvent>(resizedEvent);
		return 0;
	}
	// Set min window size to prevent it from becoming too small
	case WM_GETMINMAXINFO:
		if (windowManager) {
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = windowManager->min_width;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = windowManager->min_height;
		}
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		auto mouseDownEvent = MouseDownEvent(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), windowManager->m_hMainWnd);
		EventManager().NotifyListeners<MouseDownEvent>(mouseDownEvent);
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		auto mouseUpEvent = MouseUpEvent(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), windowManager->m_hMainWnd);
		EventManager().NotifyListeners<MouseUpEvent>(mouseUpEvent);
		return 0;
	case WM_MOUSEMOVE:
		auto mouseMoveEvent = MouseMoveEvent(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), windowManager->m_hMainWnd);
		EventManager().NotifyListeners<MouseMoveEvent>(mouseMoveEvent);
		return 0;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM WindowManager::RegisterWindow()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInst;
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_DX12BOXTUTORIAL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DX12BOXTUTORIAL);
	wcex.lpszClassName = m_title.c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL WindowManager::InitializeWindow()
{
	m_hMainWnd = CreateWindowW(m_title.c_str(), m_title.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, m_width, m_height, nullptr, nullptr, m_hInst, this);

	return !!m_hMainWnd;
}

BOOL WindowManager::ShowAppWindow()
{
	return ShowWindow(m_hMainWnd, m_cmdShow) == 0;
}

BOOL WindowManager::UpdateAppWindow()
{
	return UpdateWindow(m_hMainWnd);
}

