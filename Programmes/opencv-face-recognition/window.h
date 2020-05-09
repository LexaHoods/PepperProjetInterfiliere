#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <string>


class Window
{
public:
	bool create(const char* className, const char* title, int width, int height, DWORD flags);
	virtual LRESULT proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) = 0;
	HWND getHandle();
	void show();

protected:
	HWND m_hwnd;

private:
	bool registerClass(const char* className);
	static LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};


#endif // WINDOW_H
