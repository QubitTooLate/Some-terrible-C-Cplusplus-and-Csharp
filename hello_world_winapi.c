
#include <Windows.h>

#define CLASS_NAME L"CLASS_WindowsHelloWorld"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);

		FillRect(hDC, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		DrawTextExW(hDC, L"Hello, World!", -1, &ps.rcPaint, DT_CENTER | DT_VCENTER | DT_SINGLELINE, NULL);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	return 0;
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WNDCLASSEXW wndClass;
	memset(&wndClass, 0, sizeof wndClass);
	wndClass.cbSize = sizeof wndClass;
	wndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
	wndClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = WndProc;
	wndClass.lpszClassName = CLASS_NAME;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;

	if (RegisterClassExW(&wndClass))
	{
		HWND hWnd = CreateWindowExW(
			0,
			CLASS_NAME,
			L"Windows Hello, World!",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0,
			CW_USEDEFAULT, 0,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (hWnd)
		{
			ShowWindow(hWnd, nCmdShow);

			MSG msg;
			BOOL succes;
			while ((succes = GetMessageW(&msg, (HWND)NULL, 0, 0)) != 0 && succes != -1)
			{
				DispatchMessageW(&msg);
			}

			return msg.wParam;

			UNREFERENCED_PARAMETER(pCmdLine);
			UNREFERENCED_PARAMETER(hPrevInstance);
		}
	}

	return 0;
}
