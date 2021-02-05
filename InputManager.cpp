#include "InputManager.h"
#include "EventManager.h"

InputManager::InputManager(ICamera& camera) :
	m_camera{ camera }
{
}

void InputManager::OnKeyboardInput(double deltaTime, ICamera& camera)
{
	if (GetAsyncKeyState('W') & 0x8000)
		camera.Walk(10.0f * deltaTime);

	if (GetAsyncKeyState('S') & 0x8000)
		camera.Walk(-10.0f * deltaTime);

	if (GetAsyncKeyState('A') & 0x8000)
		camera.Strafe(-10.0f * deltaTime);

	if (GetAsyncKeyState('D') & 0x8000)
		camera.Strafe(10.0f * deltaTime);
}

void InputManager::CaptureMouseInputs()
{
	EventManager().AddListener<MouseDownEvent>(mouseDownEventName,
		[&](MouseDownEvent& event) {
			OnMouseDown(event.btnState, event.x, event.y, event.hWnd);
		});

	EventManager().AddListener<MouseUpEvent>(mouseUpEventName,
		[&](MouseUpEvent& event) {
			OnMouseUp(event.btnState, event.x, event.y, event.hWnd);
		});

	EventManager().AddListener<MouseMoveEvent>(mouseMoveEventName,
		[&](MouseMoveEvent& event) {
			OnMouseMove(event.btnState, event.x, event.y, event.hWnd);
		});
}

void InputManager::UncaptureMouseInputs()
{
}

void InputManager::OnMouseDown(WPARAM btnState, int x, int y, HWND hWnd)
{
	m_prevMousePos.x = x;
	m_prevMousePos.y = y;

	SetCapture(hWnd);
}

void InputManager::OnMouseUp(WPARAM btnState, int x, int y, HWND hWnd)
{
	ReleaseCapture();
}

void InputManager::OnMouseMove(WPARAM btnState, int x, int y, HWND hWnd)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - m_prevMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - m_prevMousePos.y));

		m_camera.Pitch(dy);
		m_camera.RotateWorldY(dx);
	}

	m_prevMousePos.x = x;
	m_prevMousePos.y = y;
}
