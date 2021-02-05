#include <functional>
#include <unordered_map>
#include <Windows.h>
#include <windowsx.h>
#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

struct WindowResizedEvent;

class EventManager
{
public:
	template <typename T>
	static void AddListener(std::string id, std::function<void(T&)> callback) {
		getListeners<T>()[id] = callback;
	}

	template <typename T>
	static void NotifyListeners(T& event) {
		for (auto f : getListeners<T>()) {
			f.second(event);
		}
	};

private:
	template <typename T>
	static std::unordered_map<std::string, std::function<void(T&)>>& getListeners() {
		static std::unordered_map<std::string, std::function<void(T&)>> listeners;
		return listeners;
	}
};

// Events
struct WindowResizedEvent
{
public:
	int width = 0;
	int height = 0;

	WindowResizedEvent() = default;
	WindowResizedEvent(int width, int height) : width(width), height(height){}
};


struct MouseDownEvent
{
public:
	WPARAM btnState;
	int x;
	int y;
	HWND hWnd;

	MouseDownEvent() = default;
	MouseDownEvent(WPARAM btnState, int x, int y, HWND hWnd) : btnState(btnState), x(x), y(y), hWnd{ hWnd }{}
};

struct MouseUpEvent
{
public:
	WPARAM btnState;
	int x;
	int y;
	HWND hWnd;

	MouseUpEvent() = default;
	MouseUpEvent(WPARAM btnState, int x, int y, HWND hWnd) : btnState(btnState), x(x), y(y), hWnd{ hWnd }{}
};

struct MouseMoveEvent
{
public:
	WPARAM btnState;
	int x;
	int y;
	HWND hWnd;

	MouseMoveEvent() = default;
	MouseMoveEvent(WPARAM btnState, int x, int y, HWND hWnd) : btnState(btnState), x(x), y(y), hWnd{ hWnd }{}
};

#endif
