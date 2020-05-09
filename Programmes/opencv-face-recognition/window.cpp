#include "window.h"


bool Window::create(const char* className, const char* title, int width, int height, DWORD flags)
{
	if (!registerClass(className))
		return false;

	m_hwnd = CreateWindow(className, title, flags, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, GetModuleHandle(NULL), this);

	return (bool)(m_hwnd);
}

HWND Window::getHandle()
{
	return m_hwnd;
}

void Window::show()
{
	if (m_hwnd)
		ShowWindow(m_hwnd, SW_SHOW);
}

bool Window::registerClass(const char* className)
{
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszMenuName = 0;
	wc.lpszClassName = className;
	wc.lpfnWndProc = wndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.cbWndExtra = sizeof(Window*);
	wc.cbClsExtra = 0;
	wc.style = 0;

	return !!RegisterClass(&wc);
}

LRESULT Window::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static Window *window = nullptr;

	if (msg == WM_CREATE) {
		window = (Window*)((LPCREATESTRUCT)lp)->lpCreateParams;
		if (window)
			window->m_hwnd = hwnd;
	}

	if (window != nullptr)
		return window->proc(hwnd, msg, wp, lp);

	return DefWindowProc(hwnd, msg, wp, lp);
}
