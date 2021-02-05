#ifndef INPUTMANAGER_H_ 
#define INPUTMANAGER_H_

#include <Windows.h>
#include <WinUser.h>
#include "ICamera.h"
#include <string>

class InputManager
{
public:
	InputManager(ICamera& camera);

	void OnKeyboardInput(double deltaTime, ICamera& camera);
	void CaptureMouseInputs();
	void UncaptureMouseInputs();

private:
	const std::string mouseDownEventName{ "InputManagerMouseDownSubscriber" };
	const std::string mouseUpEventName{ "InputManagerMouseUpSubscriber" };
	const std::string mouseMoveEventName{ "InputManagerMouseMoveSubscriber" };

	ICamera& m_camera;
	POINT m_prevMousePos;

	void OnMouseDown(WPARAM btnState, int x, int y, HWND hWnd);
	void OnMouseUp(WPARAM btnState, int x, int y, HWND hWnd);
	void OnMouseMove(WPARAM btnState, int x, int y, HWND hWnd);
};


#endif
