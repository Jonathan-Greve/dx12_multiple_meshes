#ifndef WINDOW_MANAGER_H_
#define WINDOW_MANAGER_H_

#include <wtypes.h>
#include <Windows.h>
#include <string>

constexpr unsigned int kMaxLoadString = 100;

class WindowManager
{
public:
	WindowManager(HINSTANCE hInstance, int cmdShow, int height, int width, std::wstring title);
	BOOL CreateMainWindow();

	int GetHeight();
	int GetWidth();
	const std::wstring& GetTitle();
	const HWND& GetMainWindow();

	void UpdateHeight(int newHeight);
	void UpdateWidth(int newWidth);
	void UpdateDimensions(int newHeight, int newWidth);
	void UpdateTitle(const std::wstring& newHeight);

	bool isRezising = false;

private:
	const int min_width = 300;
	const int min_height = 300;

	HINSTANCE m_hInst; // current instance
	int m_cmdShow;
	std::wstring m_title; // The title bar text
	int m_height;
	int m_width;
	HWND m_hMainWnd;

	ATOM RegisterWindow();
	BOOL InitializeWindow();
	BOOL ShowAppWindow();
	BOOL UpdateAppWindow();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

#endif // WINDOW_MANAGER_H_


