//#define EPS (0.0000001)

#include <Windows.h>
#include <tchar.h>
#include <list>
#include "Vec2.h"
#include "GameObject.h"
#include "GameStates.h"

using namespace std;

GameStateContainer states;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR cmd, int ss)
{
	WNDCLASSEX wc;
	MSG msg;
	HWND hwnd;
	TCHAR szClassName[] = _T("Arcanoid Game WNDClass");

#ifdef _DEBUG
	_CrtMemState _ms;
	_CrtMemCheckpoint(&_ms);
#endif

	GSMainMenu *main_state = new GSMainMenu(&states);
	states.push_state(main_state);

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpszClassName = szClassName;
	wc.lpfnWndProc = WndProc;

	RegisterClassEx(&wc);

	hwnd = CreateWindowEx(NULL, szClassName, _T("Arcanoid"),
		WS_POPUP, 
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, SW_MAXIMIZE);
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
	SetTimer(hwnd, 1, 10, NULL);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	KillTimer(hwnd, 1);
	states.clear();

#ifdef _DEBUG
	_CrtMemDumpAllObjectsSince(&_ms);
#endif

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	HDC hmem_result_dc;
	HBITMAP hbmp_new_result;
	HBITMAP hbmp_old_result;
	PAINTSTRUCT ps;
	RECT rc;
	int real_width;

	switch (msg)
	{
	case WM_SIZE:
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		hmem_result_dc = CreateCompatibleDC(hdc);
		hbmp_new_result = CreateCompatibleBitmap(hdc, FRAME_X, FRAME_Y);
		hbmp_old_result = (HBITMAP)SelectObject(hmem_result_dc, hbmp_new_result);
		
		if (states.size() > 0)
			states.current()->render(hmem_result_dc);

		GetClientRect(hwnd, &rc);
		real_width = rc.bottom / 3 * 4;
		StretchBlt(hdc, (rc.right - real_width) / 2, 0, real_width, rc.bottom,
			hmem_result_dc, 0, 0, FRAME_X, FRAME_Y, SRCCOPY);

		SelectObject(hmem_result_dc, hbmp_old_result);
		DeleteObject(hbmp_new_result);
		DeleteDC(hmem_result_dc);

		EndPaint(hwnd, &ps);
		return TRUE;

	case WM_LBUTTONUP:
	{
		GetClientRect(hwnd, &rc);
		states.current()->input(msg, wParam, 
			AbstractGameState::ConvertCoord(lParam, rc));
	}
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_TIMER:
		if (states.size() > 0)
			states.current()->input(msg, wParam, lParam);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	states.update_state();
	if (states.size() > 0)
		InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}